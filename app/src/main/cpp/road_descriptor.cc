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

#include <mesh_generated.h>


RoadDescriptor::RoadDescriptor(fplbase::AssetManager *asset_manager) {
  LoadMeshData(asset_manager->LoadFileAsset("meshes/road.fplmesh"));
}


void RoadDescriptor::LoadMeshData(fplbase::FileAsset* file) {
  const std::string& road_flatbuf = file->contents;
  flatbuffers::Verifier verifier(
      reinterpret_cast<const uint8_t *>(road_flatbuf.c_str()), road_flatbuf.length());
  assert(meshdef::VerifyMeshBuffer(verifier));

  fplbase::Mesh::ParseInterleavedVertexData(road_flatbuf.c_str(), &vertex_data_);
}
