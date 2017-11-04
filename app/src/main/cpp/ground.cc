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

#include "ground.h"

namespace {

struct Vertex {
  float x;
  float y;
  float z;
  float u;
  float v;
};

constexpr fplbase::Attribute kFormat[] = {
    fplbase::Attribute::kPosition3f,
    fplbase::Attribute::kTexCoord2f,
    fplbase::Attribute::kEND
};

constexpr float kSize = 500.0f;

constexpr Vertex kVerticesData[] = {
    {-kSize, 0.0f, -kSize, 0.0f, 0.0f},
    { kSize, 0.0f, -kSize, 100.0f, 0.0f},
    { kSize, 0.0f,  kSize, 100.0f, 100.0f},
    {-kSize, 0.0f,  kSize, 0.0f, 100.0f}
};

constexpr uint16_t kVerticesCount = 4;

constexpr uint16_t kIndices[] = {
    0, 1, 3,
    1, 2, 3,
};

constexpr uint16_t kIndicesCount = 6;

const mathfu::mat4 kGroundTranslationDelta = mathfu::mat4::FromTranslationVector({0.0f, -0.02f, 0.0f});

}  // namespace


Ground::Ground(fplbase::AssetManager* asset_manager)
    : asset_manager_(asset_manager),
      ground_mesh_(kVerticesData, kVerticesCount, sizeof(Vertex), kFormat),
      ground_material_(),
      road_mesh_(asset_manager->LoadMesh("meshes/road.fplmesh")),
      shader_(asset_manager->LoadShader("shaders/textured")) {
  fplbase::Texture* ground_texture = asset_manager->LoadTexture("textures/ground.webp");
  ground_material_.textures().push_back(ground_texture);
  ground_mesh_.AddIndices(kIndices, kIndicesCount, &ground_material_);
  fplbase::Texture* road_texture = asset_manager->LoadTexture("textures/road.webp");
  road_mesh_->GetMaterial(0)->textures().push_back(road_texture);
}


void Ground::Render(fplbase::Renderer* renderer,
                    const mathfu::mat4& model_view_projection_matrix) {
  if (!asset_manager_->TryFinalize())
    return;
  renderer->set_model_view_projection(model_view_projection_matrix * kGroundTranslationDelta);
  renderer->SetShader(shader_);
  renderer->Render(&ground_mesh_);
  renderer->set_model_view_projection(model_view_projection_matrix);
  renderer->SetShader(shader_);
  renderer->Render(road_mesh_);
}
