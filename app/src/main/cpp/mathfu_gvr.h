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

#ifndef VRAZE_MATHFU_GVR_H_
#define VRAZE_MATHFU_GVR_H_

#include <mathfu/vector.h>
#include <mathfu/matrix.h>
#include <vr/gvr/capi/include/gvr_types.h>


inline mathfu::vec2i GvrToMathfu(const gvr::Sizei &size) {
  return {size.width, size.height};
}


inline mathfu::mat4 GvrToMathfu(const gvr::Mat4f &mat) {
  mathfu::mat4 result;
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      result(i, j) = mat.m[i][j];
  return result;
}


inline mathfu::quat GvrToMathfu(const gvr::Quatf &quat) {
  return {quat.qw, quat.qx, quat.qy, quat.qz};
}

#endif //VRAZE_MATHFU_GVR_H_
