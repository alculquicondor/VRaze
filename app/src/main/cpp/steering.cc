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
 
#include "steering.h"


namespace {

const mathfu::mat4 kModelMatrix = mathfu::mat4::FromScaleVector({0.07f, 0.07f, 0.07f});

}


Steering::Steering(fplbase::AssetManager* asset_manager) :
    mesh_(asset_manager->LoadMesh("meshes/steering.fplmesh")),
    shader_(asset_manager->LoadShader("shaders/colored")) {
}


void Steering::Render(fplbase::Renderer* renderer,
                      const mathfu::mat4& model_view_projection_matrix) {
  renderer->set_model_view_projection(model_view_projection_matrix * kModelMatrix);
  renderer->SetShader(shader_);
  renderer->Render(mesh_, true);
}
