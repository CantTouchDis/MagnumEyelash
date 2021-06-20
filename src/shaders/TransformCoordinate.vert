layout(location = 0) in vec4 position;

uniform mat4 transformationMatrix;
uniform mat4 projectionMatrix;

void main() {
  gl_Position = projectionMatrix * transformationMatrix * position;
}
