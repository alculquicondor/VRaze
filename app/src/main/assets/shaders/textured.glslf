#version 300 es

in highp vec2 vTexCoord;

out vec4 frag_color;

uniform sampler2D texture_unit_0;

void main() {
  frag_color = texture(texture_unit_0, mod(vTexCoord, 1.0));
}