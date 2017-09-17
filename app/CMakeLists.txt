# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

cmake_minimum_required(VERSION 3.4.1)

include(AndroidNdkModules)
include(ExternalProject)

set(fplutil_dir ${CMAKE_SOURCE_DIR}/../dependencies/fplutil)
include("${fplutil_dir}/buildutil/cmake_common.txt")
set_common_build_variables()

ExternalProject_Add(
    fpl
    SOURCE_DIR ${CMAKE_SOURCE_DIR}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND env APP_PLATFORM=${ANDROID_PLATFORM} FPLUTIL_DIR=${fplutil_dir} ${ANDROID_NDK}/ndk-build
    INSTALL_COMMAND ""
)

include_directories(
    ../dependencies/gvr-android-sdk/libraries/headers
    ${dependencies_fplbase_dir}/include
)

link_directories(
    ../libraries/jni/armeabi-v7a
)

add_library(
    vraze_jni SHARED
    src/main/cpp/vraze_jni.cpp
)

add_dependencies(vraze_jni fpl)

find_library(log-lib log)

target_link_libraries(
    vraze_jni

    android ${log-lib}
    EGL GLESv2
    gvr gvr_audio
    fplbase_stdlib
)