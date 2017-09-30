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
 
#include "car.h"

#include "log_utils.h"


constexpr float Car::WEIGHT = 1500.0f;
constexpr float Car::MIN_TRACTION = 1500.0f;
constexpr float Car::MAX_TRACTION = 30000.0f;
constexpr float Car::TRACTION_INCREASE = 2000.0f;
constexpr float Car::DRAG_RATIO = 7.0f;
constexpr float Car::FRICTION_RATIO = 300.0f;


Car::Car(const mathfu::vec2 position)
    : position_(position), traction_(MIN_TRACTION) {
}


void Car::Move(float delta_time, bool accelerating) {
  position_ += direction_ * speed_ * delta_time;

  float force = -DRAG_RATIO * speed_ * speed_ - FRICTION_RATIO * speed_;
  if (accelerating) {
    traction_ += delta_time * TRACTION_INCREASE;
    traction_ = std::min(traction_, MAX_TRACTION);
    force += traction_;
  } else {
    traction_ -= delta_time * TRACTION_INCREASE;
    traction_ = std::max(traction_, MIN_TRACTION);
  }
  speed_ += force / WEIGHT * delta_time;
}
