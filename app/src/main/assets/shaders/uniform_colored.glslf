#version 300 es

in lowp vec4 color;

out vec4 frag_color;

uniform sampler2D texture_unit_0;

void main() {
  frag_color = color;
}