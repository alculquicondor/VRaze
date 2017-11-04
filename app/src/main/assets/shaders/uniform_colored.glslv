#version 300 es

#define PHONG_SHADING

#include "shaders/fplbase/phong_shading.glslf_h"

in vec4 aPosition;
in vec3 aNormal;
uniform vec3 light_pos;    //in object space

out lowp vec4 color;

uniform mat4 model_view_projection;


void main() {
  vec4 position = model_view_projection * aPosition;

  gl_Position = position;

  vec3 light_direction = CalculateLightDirection(position.xyz, light_pos);
  lowp vec4 shading_tint = CalculatePhong(position.xyz, aNormal,
                                          light_direction);
  color = vec4(0.8, 0.1, 0.1, 1.0) * shading_tint;
}