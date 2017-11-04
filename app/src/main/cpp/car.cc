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

namespace {

const mathfu::mat4 kModelMatrix = mathfu::mat4::FromTranslationVector({0.0f, -0.82f, 0.0f}) *
    mathfu::mat4::FromScaleVector({0.07f, 0.07f, 0.07f});

}

Car::Car(fplbase::AssetManager *asset_manager) :
    mesh_(asset_manager->LoadMesh("meshes/car.fplmesh")),
    shader_(asset_manager->LoadShader("shaders/uniform_colored")) {
}

void Car::Render(fplbase::Renderer *renderer, const mathfu::mat4 &model_view_projection_matrix) {
  shader_->SetUniform("ambient_material", mathfu::vec4{0.2f, 0.2f, 0.2f, 1.0f});
  shader_->SetUniform("diffuse_material", mathfu::vec4{0.8f, 0.8f, 0.8f, 1.0f});
  renderer->set_model_view_projection(model_view_projection_matrix * kModelMatrix);
  renderer->SetShader(shader_);
  renderer->Render(mesh_, true);
}
