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

#ifndef VRAZE_ROAD_DESCRIPTOR_H_
#define VRAZE_ROAD_DESCRIPTOR_H_

#include <vector>

#include <mesh_generated.h>
#include <fplbase/asset_manager.h>
#include <fplbase/material.h>

class RoadDescriptor {
 public:
  RoadDescriptor(fplbase::AssetManager* asset_manager);

  bool IsInside(mathfu::vec2 position) const;

 private:
  fplbase::Mesh::InterleavedVertexData vertex_data_;
  const meshdef::Surface* surface_;
  std::vector<mathfu::vec2> triangles_;

  void LoadMeshData(fplbase::FileAsset* file);
  void ProcessBorders();
};

#endif //VRAZE_ROAD_DESCRIPTOR_H_
