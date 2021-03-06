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

#include "vraze_jni.h"

#include "vraze_app.h"
#include "log_utils.h"

namespace {
inline jlong jptr(VRazeApp *ptr) { return reinterpret_cast<intptr_t>(ptr); }

inline VRazeApp *ptr(jlong jptr) { return reinterpret_cast<VRazeApp *>(jptr); }

}  // namespace

NATIVE_METHOD(jlong, nativeOnCreate)
(JNIEnv *env, jobject obj, jobject class_loader, jobject android_context, jobject asset_mgr,
 jlong gvr_context_ptr, jboolean is_multiplayer, jint playerNumber) {
  auto audio_api = std::make_unique<gvr::AudioApi>();
  audio_api->Init(env, android_context, class_loader, GVR_AUDIO_RENDERING_BINAURAL_HIGH_QUALITY);
  return jptr(new VRazeApp(env, asset_mgr, gvr_context_ptr, std::move(audio_api),
                           is_multiplayer, playerNumber));
}

NATIVE_METHOD(void, nativeOnResume)
(JNIEnv *env, jobject obj, jlong controller_paint_jptr) {
  ptr(controller_paint_jptr)->OnResume();
}

NATIVE_METHOD(void, nativeOnPause)
(JNIEnv *env, jobject obj, jlong controller_paint_jptr) {
  ptr(controller_paint_jptr)->OnPause();
}

NATIVE_METHOD(void, nativeOnSurfaceCreated)
(JNIEnv *env, jobject obj, jlong controller_paint_jptr) {
  ptr(controller_paint_jptr)->OnSurfaceCreated();
}

NATIVE_METHOD(void, nativeOnSurfaceChanged)
(JNIEnv *env, jobject obj, jint width, jint height,
 jlong controller_paint_jptr) {
  ptr(controller_paint_jptr)
      ->OnSurfaceChanged(static_cast<int>(width), static_cast<int>(height));
}

NATIVE_METHOD(void, nativeOnDrawFrame)
(JNIEnv *env, jobject obj, jlong controller_paint_jptr) {
  ptr(controller_paint_jptr)->OnDrawFrame();
}

NATIVE_METHOD(void, nativeOnDestroy)
(JNIEnv *env, jobject obj, jlong controller_paint_jptr) {
  delete ptr(controller_paint_jptr);
}


NATIVE_METHOD_RT(void, nativeMoveCar)
(JNIEnv* env, jobject obj, jlong controller_paint_jptr,float x, float y, float dir_x, float dir_y) {
  ptr(controller_paint_jptr)->OnMoveOpponent(x, y, dir_x, dir_y);
}

