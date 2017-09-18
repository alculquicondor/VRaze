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

constexpr int64_t kPredictionTimeWithoutVsyncNanos = 50000000;  // 50ms

const mathfu::vec4 kSkyColor{0.8f, 0.8f, 1.0f, 1.0f};

constexpr float kNearClip = 0.1f;
constexpr float kFarClip = 1000.0f;

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


VRazeApp::VRazeApp(JNIEnv *env, jobject asset_manager, jlong gvr_context_ptr)
    : gvr_context_(reinterpret_cast<gvr_context*>(gvr_context_ptr)),
      gvr_api_(gvr::GvrApi::WrapNonOwned(gvr_context_)),
      gvr_api_initialized_(false),
      viewport_list_(gvr_api_->CreateEmptyBufferViewportList()),
      scene_viewport_(gvr_api_->CreateBufferViewport()),
      a_asset_mgr_(AAssetManager_fromJava(env, asset_manager)) {
  CHECK(a_asset_mgr_);
  fplbase::SetAAssetManager(a_asset_mgr_);
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
  if (controller_api_)
    controller_api_->Resume();
}


void VRazeApp::OnPause() {
  LOGD("VRazeApp::OnPause");
  if (gvr_api_initialized_)
    gvr_api_->PauseTracking();
  if (controller_api_)
    controller_api_->Pause();
}


void VRazeApp::OnSurfaceCreated() {
  LOGD("VRazeApp::OnSurfaceCreated");

  LOGD("Initializing GL on GvrApi.");
  gvr_api_->InitializeGl();

  LOGD("Initializing Controller Api.");
  controller_api_ = std::make_unique<gvr::ControllerApi>();
  CHECK(controller_api_);
  if (controller_api_->Init(gvr::ControllerApi::DefaultOptions(), gvr_context_)) {
    LOGI("Controller support available.");
  } else {
    LOGI("No controller support available.");
    controller_api_ = nullptr;
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
  fpl_asset_manager_ = std::make_unique<fplbase::AssetManager>(*renderer_);

  scene_ = std::make_unique<Scene>();
  scene_->SetUp(fpl_asset_manager_.get());

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
  const mathfu::mat4 head_view = GvrToMathfu(head_view_gvr);
  const mathfu::mat4 left_eye_view =
      GvrToMathfu(gvr_api_->GetEyeFromHeadMatrix(GVR_LEFT_EYE)) * head_view;
  const mathfu::mat4 right_eye_view =
      GvrToMathfu(gvr_api_->GetEyeFromHeadMatrix(GVR_RIGHT_EYE)) * head_view;

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
  scene_->Render(renderer_.get(), proj_matrix * eye_view_matrix);
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
  // renderer_->ScissorOn({left, bottom}, {width, height});
}
