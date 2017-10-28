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
 
#include "road_descriptor.h"


namespace {

inline float Sign(mathfu::vec2 p1, mathfu::vec2 p2, mathfu::vec2 p3) {
  return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

}  // namespace


RoadDescriptor::RoadDescriptor(fplbase::AssetManager *asset_manager) {
  LoadMeshData(asset_manager->LoadFileAsset("meshes/road.fplmesh"));
  ProcessBorders();
}


void RoadDescriptor::LoadMeshData(fplbase::FileAsset* file) {
  const std::string& road_flatbuf = file->contents;
  flatbuffers::Verifier verifier(
      reinterpret_cast<const uint8_t *>(road_flatbuf.c_str()), road_flatbuf.length());
  assert(meshdef::VerifyMeshBuffer(verifier));
  const meshdef::Mesh* meshdef = meshdef::GetMesh(road_flatbuf.c_str());
  assert(meshdef->surfaces()->size() == 1);
  surface_ = meshdef->surfaces()->Get(0);

  fplbase::Mesh::ParseInterleavedVertexData(road_flatbuf.c_str(), &vertex_data_);
}


void RoadDescriptor::ProcessBorders() {
  const flatbuffers::Vector<uint16_t> *indices = surface_->indices();
  for (uint16_t index : *indices) {
    assert(index < vertex_data_.count);
    auto position = reinterpret_cast<const mathfu::vec3 *>(
        reinterpret_cast<const uint8_t*>(vertex_data_.vertex_data) + index * vertex_data_.vertex_size);
    triangles_.push_back({-position->z, -position->x});
  }
}


bool RoadDescriptor::IsInside(mathfu::vec2 position) const {
  bool b;
  for (size_t i = 0; i < triangles_.size(); i += 3) {
    b = Sign(position, triangles_[i], triangles_[i + 1]) < 0.0f;
    if (b != (Sign(position, triangles_[i + 1], triangles_[i + 2]) < 0.0f))
      continue;
    if (b == (Sign(position, triangles_[i + 2], triangles_[i]) < 0.0f))
      return true;
  }
  return false;
}
