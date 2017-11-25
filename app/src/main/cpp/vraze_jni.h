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

#ifndef VRAZE_VRAZE_JNI_H_
#define VRAZE_VRAZE_JNI_H_

#include <jni.h>
#include <stdint.h>

#define NATIVE_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL                 \
      Java_pe_edu_ucsp_vraze_MainActivity_##method_name

#define NATIVE_METHOD_RT(return_type, method_name) \
  JNIEXPORT return_type JNICALL                 \
      Java_pe_edu_ucsp_vraze_RealTime_##method_name

extern "C" {

NATIVE_METHOD(jlong, nativeOnCreate)
(JNIEnv* env, jobject obj, jobject class_loader, jobject android_context, jobject asset_mgr,
 jlong gvrContextPtr, jboolean is_multiplayer, jint player_number);
NATIVE_METHOD(void, nativeOnResume)
(JNIEnv* env, jobject obj, jlong controller_paint_jptr);
NATIVE_METHOD(void, nativeOnPause)
(JNIEnv* env, jobject obj, jlong controller_paint_jptr);
NATIVE_METHOD(void, nativeOnSurfaceCreated)
(JNIEnv* env, jobject obj, jlong controller_paint_jptr);
NATIVE_METHOD(void, nativeOnSurfaceChanged)
(JNIEnv* env, jobject obj, jint width, jint height,
 jlong controller_paint_jptr);
NATIVE_METHOD(void, nativeOnDrawFrame)
(JNIEnv* env, jobject obj, jlong controller_paint_jptr);
NATIVE_METHOD(void, nativeOnDestroy)
(JNIEnv* env, jobject obj, jlong controller_paint_jptr);

NATIVE_METHOD_RT(void, nativeMoveCar)
(JNIEnv* env, jobject obj, jlong controller_paint_jptr,float x, float y, float dir_x, float dir_y);

}

#endif //VRAZE_VRAZE_JNI_H_
