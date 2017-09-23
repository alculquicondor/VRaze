# Copyright 2017 Aldo Culquicondor
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)/..

FPLUTIL_DIR := $(LOCAL_PATH)/../dependencies/fplutil

include $(FPLUTIL_DIR)/buildutil/android_common.mk
DEPENDENCIES_GVR_SDK_DIR := $(LOCAL_PATH)/../dependencies/gvr-android-sdk

include $(DEPENDENCIES_FLATBUFFERS_DIR)/android/jni/include.mk

include $(CLEAR_VARS)
LOCAL_MODULE := vraze_jni
LOCAL_CPPFLAGS := -DFPLBASE_BACKEND_STDLIB
LOCAL_C_INCLUDES := \
  $(DEPENDENCIES_FPLUTIL_DIR)/libfplutil/include \
  $(DEPENDENCIES_FPLBASE_DIR)/include \
  $(DEPENDENCIES_MATHFU_DIR)/include \
  $(DEPENDENCIES_GVR_SDK_DIR)/libraries/headers \
  $(LOCAL_PATH)/src

LOCAL_SRC_FILES := \
  src/main/cpp/ground.cc \
  src/main/cpp/scene.cc \
  src/main/cpp/steering.cc \
  src/main/cpp/vraze_jni.cc \
  src/main/cpp/vraze_app.cc

LOCAL_STATIC_LIBRARIES := \
  libfplbase_stdlib

LOCAL_SHARED_LIBRARIES :=

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_LDLIBS := -L$(LOCAL_PATH)/../libraries/jni/armeabi-v7a
endif
ifeq ($(TARGET_ARCH_ABI),x86)
LOCAL_LDLIBS := -L$(LOCAL_PATH)/../libraries/jni/x86
endif

LOCAL_LDLIBS += -ldl -lgvr -lGLESv3

include $(BUILD_SHARED_LIBRARY)

$(call import-add-path,$(DEPENDENCIES_FLATBUFFERS_DIR)/..)
$(call import-add-path,$(DEPENDENCIES_FPLBASE_DIR)/..)
$(call import-add-path,$(DEPENDENCIES_MATHFU_DIR)/..)

$(call import-module,flatbuffers/android/jni)
$(call import-module,fplbase/jni)
$(call import-module,mathfu/jni)
