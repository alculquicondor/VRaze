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

const mathfu::mat4 kModelMatrix = mathfu::mat4::FromTranslationVector({0.0f, -0.85f, 0.0f}) *
    mathfu::mat4::FromScaleVector({0.07f, 0.07f, 0.07f});

const mathfu::vec4 kAmbientMaterial = {0.7f, 0.7f, 0.7f, 1.0f};
const mathfu::vec4 kDiffuseMaterial = {0.4f, 0.4f, 0.4f, 1.0f};

}

Car::Car(fplbase::AssetManager *asset_manager, const mathfu::vec4& color) :
    color_(color),
    mesh_(asset_manager->LoadMesh("meshes/car.fplmesh")),
    shader_(asset_manager->LoadShader("shaders/uniform_colored")) {
}

void Car::Render(fplbase::Renderer *renderer, const mathfu::mat4 &model_view_projection_matrix) {
  shader_->SetUniform("ambient_material", kAmbientMaterial);
  shader_->SetUniform("diffuse_material", kDiffuseMaterial);
  renderer->set_model_view_projection(model_view_projection_matrix * kModelMatrix);
  renderer->SetShader(shader_);
  renderer->set_color(color_);
  renderer->Render(mesh_, true);
}
