layout(location = 0) in vec4 position;
layout(location = 1) in float w;


out float width_vs;

void main() {
  gl_Position = position;
  width_vs = w;
}
