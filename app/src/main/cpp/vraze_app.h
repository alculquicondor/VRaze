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

#ifndef VRAZE_VRAZE_APP_H_
#define VRAZE_VRAZE_APP_H_

#include <android/asset_manager.h>
#include <fplbase/renderer.h>
#include <jni.h>
#include <vr/gvr/capi/include/gvr.h>
#include <vr/gvr/capi/include/gvr_controller.h>

#include "scene.h"


class VRazeApp {
 public:
  explicit VRazeApp(JNIEnv *env, jobject asset_manager, jlong gvr_context_ptr);
  ~VRazeApp();
  void OnResume();
  void OnPause();
  void OnSurfaceCreated();
  void OnSurfaceChanged(int width, int height);
  void OnDrawFrame();

 private:
  void PrepareFramebuffer();
  void GetInput();
  void DrawEye(gvr::Eye which_eye, const mathfu::mat4 &eye_view_matrix,
               const gvr::BufferViewport &viewport);
  void SetUpViewPortAndScissor(const gvr::Sizei& framebuf_size,
                               const gvr::BufferViewport& params);
  void UpdateSteeringRotation(float steering_rotation);

  gvr_context* gvr_context_;
  std::unique_ptr<gvr::GvrApi> gvr_api_;
  bool gvr_api_initialized_;
  std::unique_ptr<gvr::ControllerApi> controller_api_;
  std::unique_ptr<gvr::SwapChain> swap_chain_;
  gvr::BufferViewportList viewport_list_;
  gvr::BufferViewport scene_viewport_;
  gvr::Sizei framebuf_size_;
  gvr::ControllerState controller_state_;
  gvr::ClockTimePoint prev_time_point_;

  std::unique_ptr<fplbase::Renderer> renderer_;
  std::unique_ptr<fplbase::AssetManager> asset_manager_;

  std::unique_ptr<Scene> scene_;
  Car car_;

  float steering_rotation_;
  bool accelerating_;
  bool braking_;

  DISALLOW_COPY_AND_ASSIGN(VRazeApp);
};

#endif //VRAZE_VRAZE_APP_H_
