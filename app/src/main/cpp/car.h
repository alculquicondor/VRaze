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

#ifndef VRAZE_CAR_H_
#define VRAZE_CAR_H_

#include "renderable.h"

#include <fplbase/asset_manager.h>


class Car : public Renderable {
 public:
  explicit Car(fplbase::AssetManager* asset_manager, const mathfu::vec4& color);

  void Render(fplbase::Renderer* renderer,
              const mathfu::mat4& model_view_projection_matrix) override;

 private:
  fplbase::Mesh* mesh_;
  fplbase::Shader* shader_;

  mathfu::vec4 color_;
};

#endif //VRAZE_CAR_H_
