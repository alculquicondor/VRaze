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
  uint16_t u;
  uint16_t v;
};

constexpr fplbase::Attribute kFormat[] = {
    fplbase::Attribute::kPosition3f,
    fplbase::Attribute::kTexCoord2us,
    fplbase::Attribute::kEND
};

constexpr float kSize = 0.5f;

constexpr Vertex kVerticesData[] = {
    {-kSize, 0.0f, -kSize, 0, 0},
    { kSize, 0.0f, -kSize, 1, 0},
    { kSize, 0.0f,  kSize, 1, 1},
    {-kSize, 0.0f,  kSize, 0, 1}
};

constexpr uint16_t kVerticesCount = 4;

constexpr uint16_t kIndices[] = {
    0, 1, 3,
    1, 2, 3,
};

constexpr uint16_t kIndicesCount = 6;

}  // namespace


Ground::Ground()
    : mesh_(kVerticesData, kVerticesCount, sizeof(Vertex), kFormat) {
  mesh_.AddIndices(kIndices, kIndicesCount, &material_);
}


void Ground::SetUp(fplbase::AssetManager *asset_manager) {
  shader_ = asset_manager->LoadShader("shaders/textured");
}


void Ground::Render(fplbase::Renderer* renderer, const mathfu::mat4& model_view_projection_matrix) {
  renderer->set_model_view_projection(model_view_projection_matrix);
  renderer->SetShader(shader_);
  renderer->Render(&mesh_, false);
}
 