#version 300 es

in vec4 aPosition;
in vec4 aColor;

out lowp vec4 color;

uniform mat4 model_view_projection;


void main() {
  color = aColor;
  gl_Position = model_view_projection * aPosition;
}