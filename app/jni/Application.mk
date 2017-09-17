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

APP_STL := gnustl_shared
APP_ABI := armeabi-v7a
APP_CPPFLAGS += -std=c++11 -Wno-unused-function
APP_PLATFORM := android-24
APP_MODULES := vraze_jni
NDK_TOOLCHAIN_VERSION := clang