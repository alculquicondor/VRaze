#version 300 es

in vec4 aPosition;
in vec2 aTexCoord;

out mediump vec2 vTexCoord;

uniform mat4 model_view_projection;


void main() {
  vTexCoord = aTexCoord;
  gl_Position = model_view_projection * aPosition;
}