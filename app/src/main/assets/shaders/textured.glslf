#version 300 es

in mediump vec2 vTexCoord;

out vec4 frag_color;

uniform sampler2D texture_unit_0;

void main() {
  // frag_color = vec4(1.0, 1.0, 1.0, 1.0);
  frag_color = texture(texture_unit_0, vTexCoord);
}