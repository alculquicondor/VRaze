/*
 * Copyright 2017 Aldo Culquicondor
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "vraze_app.h"

#include <android/asset_manager_jni.h>
#include <fplbase/utilities.h>

#include "mathfu_gvr.h"
#include "log_utils.h"

namespace {

const std::string kEngineSounds[] = {"sounds/rocket_idle.ogg", "sounds/rocket.ogg"};
const std::string kGravelSound = "sounds/gravel.ogg";
const std::string kSoundtrackSound = "sounds/soundtrack.ogg";

constexpr int64_t kPredictionTimeWithoutVsyncNanos = 50000000;  // 50ms

const mathfu::vec4 kSkyColor{0.8f, 0.8f, 1.0f, 1.0f};

constexpr float kNearClip = 0.1f;
constexpr float kFarClip = 1000.0f;

const mathfu::quat kUnrotatedSteering =
        mathfu::quat::FromAngleAxis(M_PI_2, {-1.0f, 0.0f, 0.0f}) *
            mathfu::quat::FromAngleAxis(M_PI_2, {0.0f, 0.0f, 1.0f});

const mathfu::vec2 kStartingPositions[] = {{0.0f, 0.0f}, {0.0f, -4.0f}};

const mathfu::vec3 kEnginePosition = {0.0f, -0.2f, 1.0f};
const mathfu::vec3 kGroundPosition = {0.0f, -1.5f, 0.0f};

mathfu::mat4 PerspectiveMatrixFromView(const gvr::Rectf& fov, float near_clip, float far_clip) {
  mathfu::mat4 result;
  const float x_left = -tan(fov.left * M_PI / 180.0f) * near_clip;
  const float x_right = tan(fov.right * M_PI / 180.0f) * near_clip;
  const float y_bottom = -tan(fov.bottom * M_PI / 180.0f) * near_clip;
  const float y_top = tan(fov.top * M_PI / 180.0f) * near_clip;
  const float zero = 0.0f;

  CHECK(x_left < x_right && y_bottom < y_top && near_clip < far_clip &&
      near_clip > zero && far_clip > zero);
  const float X = (2 * near_clip) / (x_right - x_left);
  const float Y = (2 * near_clip) / (y_top - y_bottom);
  const float A = (x_right + x_left) / (x_right - x_left);
  const float B = (y_top + y_bottom) / (y_top - y_bottom);
  const float C = (near_clip + far_clip) / (near_clip - far_clip);
  const float D = (2 * near_clip * far_clip) / (near_clip - far_clip);

  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      result(i, j) = 0.0f;
    }
  }
  result(0, 0) = X;
  result(0, 2) = A;
  result(1, 1) = Y;
  result(1, 2) = B;
  result(2, 2) = C;
  result(2, 3) = D;
  result(3, 2) = -1;

  return result;
}

}  // namespace


VRazeApp::VRazeApp(JNIEnv *env, jobject asset_manager, jlong gvr_context_ptr,
                   std::unique_ptr<gvr::AudioApi> audio_api, bool multiplayer, int player_number)
    : gvr_context_(reinterpret_cast<gvr_context*>(gvr_context_ptr)),
      gvr_api_(gvr::GvrApi::WrapNonOwned(gvr_context_)),
      gvr_audio_api_(std::move(audio_api)),
      gvr_api_initialized_(false),
      viewport_list_(gvr_api_->CreateEmptyBufferViewportList()),
      scene_viewport_(gvr_api_->CreateBufferViewport()),
      steering_rotation_(0.0f),
      env_(env),
      multiplayer_(multiplayer),
      player_number_(player_number) {
  LOGD("Initializing VRazeApp.");

  fplbase::SetAAssetManager(AAssetManager_fromJava(env, asset_manager));

  for (int i = 0; i < 2; ++i) {
    gvr_audio_api_->PreloadSoundfile(kEngineSounds[i]);
    engine_sound_[i] = gvr_audio_api_->CreateSoundObject(kEngineSounds[i]);
    gvr_audio_api_->SetSoundObjectPosition(engine_sound_[i],
                                           kEnginePosition.x,
                                           kEnginePosition.y,
                                           kEnginePosition.z);
  }
  gvr_audio_api_->PreloadSoundfile(kGravelSound);
  gravel_sound_ = gvr_audio_api_->CreateSoundObject(kGravelSound);
  gvr_audio_api_->SetSoundObjectPosition(gravel_sound_,
                                         kGroundPosition.x,
                                         kGroundPosition.y,
                                         kGroundPosition.z);
  gvr_audio_api_->PreloadSoundfile(kSoundtrackSound);
  soundtrack_sound_ = gvr_audio_api_->CreateStereoSound(kSoundtrackSound);
  gvr_audio_api_->SetSoundVolume(soundtrack_sound_, 0.2f);

  gvr_audio_api_->PlaySound(engine_sound_[0], true);
  gvr_audio_api_->PlaySound(soundtrack_sound_, true);
  gvr_audio_api_->PlaySound(gravel_sound_, true);
  gvr_audio_api_->PauseSound(gravel_sound_);


  env_->GetJavaVM(&vm_);
  clazz = (jclass) env_->NewGlobalRef(env_->FindClass("pe/edu/ucsp/vraze/MainActivity"));
  if(!clazz){
    LOGD("HORROR - CANNOT FIND JAVA CLASS");
  }

  javaMethod = env_->GetStaticMethodID(clazz, "messageMe", "(FFFF)V");
  if(!javaMethod){
    LOGD("HORROR - CANNOT ACCESS JAVA METHOD");
  }

  LOGD("VRazeApp initialized.");
}


VRazeApp::~VRazeApp() {
  LOGD("VRazeApp shutdown.");
}


void VRazeApp::OnResume() {
  LOGD("VRazeApp::OnResume");
  if (gvr_api_initialized_) {
    gvr_api_->RefreshViewerProfile();
    gvr_api_->ResumeTracking();
  }
  if (gvr_controller_api_)
    gvr_controller_api_->Resume();
  if (gvr_audio_api_)
    gvr_audio_api_->Resume();
}


void VRazeApp::OnPause() {
  LOGD("VRazeApp::OnPause");
  if (gvr_api_initialized_)
    gvr_api_->PauseTracking();
  if (gvr_controller_api_)
    gvr_controller_api_->Pause();
  if (gvr_audio_api_)
    gvr_audio_api_->Pause();
}


void VRazeApp::OnSurfaceCreated() {
  LOGD("VRazeApp::OnSurfaceCreated");

  LOGD("Initializing GL on GvrApi.");
  gvr_api_->InitializeGl();
  prev_time_point_ = gvr::GvrApi::GetTimePointNow();

  LOGD("Initializing Controller Api.");
  gvr_controller_api_ = std::make_unique<gvr::ControllerApi>();
  CHECK(gvr_controller_api_);
  if (gvr_controller_api_->Init(gvr::ControllerApi::DefaultOptions(), gvr_context_)) {
    gvr_controller_api_->Resume();
    LOGI("Controller support available.");
  } else {
    LOGI("No controller support available.");
    gvr_controller_api_ = nullptr;
  }

  LOGD("Initializing Framebuffer");
  std::vector<gvr::BufferSpec> specs;
  specs.push_back(gvr_api_->CreateBufferSpec());
  framebuf_size_ = gvr_api_->GetMaximumEffectiveRenderTargetSize();

  // Because we are using 2X MSAA, we can render to half as many pixels and
  // achieve similar quality. Scale each dimension by sqrt(2)/2 ~= 7/10ths.
  framebuf_size_.width = (7 * framebuf_size_.width) / 10;
  framebuf_size_.height = (7 * framebuf_size_.height) / 10;

  specs[0].SetSize(framebuf_size_);
  specs[0].SetColorFormat(GVR_COLOR_FORMAT_RGBA_8888);
  specs[0].SetDepthStencilFormat(GVR_DEPTH_STENCIL_FORMAT_DEPTH_16);
  specs[0].SetSamples(2);
  swap_chain_ = std::make_unique<gvr::SwapChain>(gvr_api_->CreateSwapChain(specs));

  LOGD("Initializing Renderer");
  renderer_ = std::make_unique<fplbase::Renderer>();
  renderer_->Initialize(GvrToMathfu(framebuf_size_), "VRazeApp");
  fplbase::RenderState render_state;
  render_state.depth_state.test_enabled = true;
  renderer_->SetRenderState(render_state);
  asset_manager_ = std::make_unique<fplbase::AssetManager>(*renderer_);

  scene_ = std::make_unique<Scene>(asset_manager_.get());
  road_descriptor_ = std::make_unique<RoadDescriptor>(asset_manager_.get());
  car_physics_ = std::make_unique<CarPhysics>(kStartingPositions[player_number_],
                                              road_descriptor_.get());
  opponent_car_physics_ = std::make_unique<CarPhysics>(kStartingPositions[1 - player_number_],
                                                       road_descriptor_.get());
  car_physics_->SetOpponent(opponent_car_physics_.get());
  opponent_car_physics_->SetOpponent(car_physics_.get());

  LOGD("Init complete.");
}


void VRazeApp::OnSurfaceChanged(int width, int height) {
  LOGD("VRazeApp::OnSurfaceChanged %dx%d", width, height);
}


void VRazeApp::OnDrawFrame() {
  PrepareFramebuffer();

  renderer_->SetBlendMode(fplbase::BlendMode::kBlendModePreMultipliedAlpha);

  viewport_list_.SetToRecommendedBufferViewports();
  gvr::ClockTimePoint pred_time = gvr::GvrApi::GetTimePointNow();
  pred_time.monotonic_system_time_nanos += kPredictionTimeWithoutVsyncNanos;
  const gvr::Mat4f head_view_gvr = gvr_api_->GetHeadSpaceFromStartSpaceRotation(pred_time);
  gvr_audio_api_->SetHeadPose(head_view_gvr);
  gvr_audio_api_->Update();
  const mathfu::mat4 head_view = GvrToMathfu(head_view_gvr);
  const mathfu::mat4 left_eye_view =
      GvrToMathfu(gvr_api_->GetEyeFromHeadMatrix(GVR_LEFT_EYE)) * head_view;
  const mathfu::mat4 right_eye_view =
      GvrToMathfu(gvr_api_->GetEyeFromHeadMatrix(GVR_RIGHT_EYE)) * head_view;

  float delta_time = 1.0e-9f * (
      pred_time.monotonic_system_time_nanos - prev_time_point_.monotonic_system_time_nanos);
  prev_time_point_ = pred_time;

  GetInput();
  car_physics_->Move(delta_time, accelerating_, braking_, steering_rotation_);
  SendMessage(car_physics_->GetPosition()[0],car_physics_->GetPosition()[1],
              car_physics_->GetDirection()[0],car_physics_->GetDirection()[1]);

  if (not gvr_audio_api_->IsSoundPlaying(engine_sound_[accelerating_])) {
    if (accelerating_) {
      gvr_audio_api_->PauseSound(engine_sound_[0]);
      engine_sound_[1] =
          gvr_audio_api_->CreateSoundObject(kEngineSounds[1]);
      gvr_audio_api_->PlaySound(engine_sound_[1], true);
    } else {
      gvr_audio_api_->StopSound(engine_sound_[1]);
      gvr_audio_api_->ResumeSound(engine_sound_[0]);
    }
  }

  if (not car_physics_->IsOnRoad() and not gvr_audio_api_->IsSoundPlaying(gravel_sound_)) {
    gvr_audio_api_->ResumeSound(gravel_sound_);
  } else if (car_physics_->IsOnRoad() and gvr_audio_api_->IsSoundPlaying(gravel_sound_)) {
    gvr_audio_api_->PauseSound(gravel_sound_);
  }
  gvr_audio_api_->SetSoundVolume(gravel_sound_, std::min(1.0f, car_physics_->GetSpeed() / 10.0f));

  gvr::Frame frame = swap_chain_->AcquireFrame();
  frame.BindBuffer(0);

  renderer_->ClearFrameBuffer(kSkyColor);
  viewport_list_.GetBufferViewport(0, &scene_viewport_);
  DrawEye(GVR_LEFT_EYE, left_eye_view, scene_viewport_);
  viewport_list_.GetBufferViewport(1, &scene_viewport_);
  DrawEye(GVR_RIGHT_EYE, right_eye_view, scene_viewport_);
  frame.Unbind();
  frame.Submit(viewport_list_, head_view_gvr);
}


void VRazeApp::UpdateSteeringRotation(float steering_rotation) {
  if (steering_rotation > M_PI)
    steering_rotation -= 2 * M_PI;
  if (fabsf(steering_rotation - steering_rotation_) < M_PI) {
    steering_rotation_ = steering_rotation;
  }
}


void VRazeApp::GetInput() {
  if (!gvr_controller_api_) {
    // TODO: Implement input for cardboard.
    steering_rotation_ = 0.0f;
    accelerating_ = false;
    braking_ = false;
    return;
  }
  controller_state_.Update(*gvr_controller_api_);
  mathfu::quat orientation = (kUnrotatedSteering *
      GvrToMathfu(controller_state_.GetOrientation())).Normalized();
  UpdateSteeringRotation(2.0f * atan2f(orientation.vector().y, orientation.scalar()));
  accelerating_ = controller_state_.GetButtonState(gvr::kControllerButtonClick);
  braking_ = controller_state_.GetButtonState(gvr::kControllerButtonApp);
}


void VRazeApp::PrepareFramebuffer() {
  gvr::Sizei recommended_size = gvr_api_->GetMaximumEffectiveRenderTargetSize();
  recommended_size.width = (7 * recommended_size.width) / 10;
  recommended_size.height = (7 * recommended_size.height) / 10;
  if (framebuf_size_.width != recommended_size.width ||
      framebuf_size_.height != recommended_size.height) {
    swap_chain_->ResizeBuffer(0, recommended_size);
    framebuf_size_ = recommended_size;
  }
}

void VRazeApp::DrawEye(gvr::Eye which_eye,
                       const mathfu::mat4 &eye_view_matrix,
                       const gvr::BufferViewport &viewport) {
  SetUpViewPortAndScissor(framebuf_size_, viewport);
  mathfu::mat4 proj_matrix = PerspectiveMatrixFromView(
      viewport.GetSourceFov(), kNearClip, kFarClip);
  scene_->Render(renderer_.get(), proj_matrix * eye_view_matrix, *car_physics_,
                 *opponent_car_physics_, steering_rotation_);
}


void VRazeApp::SetUpViewPortAndScissor(const gvr::Sizei &framebuf_size,
                                       const gvr::BufferViewport &params) {
  const gvr::Rectf& rect = params.GetSourceUv();
  int left = static_cast<int>(rect.left * framebuf_size.width);
  int bottom = static_cast<int>(rect.bottom * framebuf_size.width);
  int width = static_cast<int>((rect.right - rect.left) * framebuf_size.width);
  int height =
      static_cast<int>((rect.top - rect.bottom) * framebuf_size.height);
  renderer_->SetViewport({left, bottom, width, height});
  // TODO: Discover how to make this work
  // renderer_->ScissorOn({left, bottom}, {width, height});
}

void VRazeApp::SendMessage(float x,float y, float dir0,float dir1){
  vm_->AttachCurrentThread(&env_, 0);
  env_->CallStaticVoidMethod(clazz, javaMethod,x,y,dir0,dir1);
}
