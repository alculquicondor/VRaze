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

#ifndef VRAZE_CAR_H_
#define VRAZE_CAR_H_

#include <mathfu/glsl_mappings.h>


class Car {
 public:
  explicit Car(const mathfu::vec2 position);
  void Move(float delta_time, bool accelerating, bool braking);

  inline const mathfu::vec2& GetPosition() const {
    return position_;
  }
 private:
  mathfu::vec2 position_;
  mathfu::vec2 direction_ = {0.0f, 1.0f};
  float speed_ = 0.0f;
  float traction_;

  static const float WEIGHT;
  static const float MIN_TRACTION;
  static const float MAX_TRACTION;
  static const float TRACTION_INCREASE;
  static const float DRAG_RATIO;
  static const float FRICTION_RATIO;
  static const float BRAKING;
};

#endif //VRAZE_CAR_H_
