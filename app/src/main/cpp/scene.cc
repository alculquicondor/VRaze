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
 
#include "scene.h"


namespace {

const float kGroundDepth = -1.0f;

const mathfu::mat4 kSteeringModelMatrix =
    mathfu::mat4::FromTranslationVector({0.0f, -0.3f, -0.4f}) *
        mathfu::mat4::FromRotationMatrix(mathfu::mat4::RotationX(-0.3f));

const mathfu::vec4 kLightPosition = {-500.0f, 2000.0f, -500.0f, 1.0f};

const mathfu::vec4 kCarColors[] = {
    {0.2f, 0.2f, 0.8f, 1.0f}, {0.8f, 0.2f, 0.2f, 1.0f}
};

}  // namespace


Scene::Scene(fplbase::AssetManager* asset_manager, int player_number)
    : car_(asset_manager, kCarColors[player_number]),
      opponent_car_(asset_manager, kCarColors[1 - player_number]),
      ground_(asset_manager), steering_(asset_manager) {
  asset_manager->StartLoadingTextures();
}


void Scene::Render(fplbase::Renderer* renderer,
                   const mathfu::mat4& view_projection_matrix,
                   const CarPhysics& car_physics,
                   const CarPhysics& opponent_car_physics,
                   float steering_rotation) {
  mathfu::mat4 world_movement =
      mathfu::mat4::FromRotationMatrix(
          mathfu::mat4::RotationY({car_physics.GetDirection().x, -car_physics.GetDirection().y})) *
          mathfu::mat4::FromTranslationVector({car_physics.GetPosition().y, 0.0f, car_physics.GetPosition().x});
  renderer->set_light_pos((world_movement * mathfu::vec4(kLightPosition)).xyz());
  ground_.Render(renderer, view_projection_matrix * world_movement *
      mathfu::mat4::FromTranslationVector({0.0f, kGroundDepth, 0.0f}));
  car_.Render(renderer, view_projection_matrix);
  steering_.Render(renderer,
                   view_projection_matrix * kSteeringModelMatrix *
                       mathfu::mat4::FromRotationMatrix(
                           mathfu::mat4::RotationZ(steering_rotation)));
  opponent_car_.Render(renderer, view_projection_matrix * world_movement *
      mathfu::mat4::FromTranslationVector({-opponent_car_physics.GetPosition().y, 0.0f,
                                           -opponent_car_physics.GetPosition().x}) *
      mathfu::mat4::FromRotationMatrix(mathfu::mat4::RotationY(
          {opponent_car_physics.GetDirection().x, opponent_car_physics.GetDirection().y})));
}
