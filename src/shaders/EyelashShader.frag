uniform vec3 color = vec3(1.0, 1.0, 1.0);

in vec4 normal;
out vec4 fragmentColor;

void main() {
  float cos = max(0, dot(normal.xyz, normalize(vec3(0, 1, 1))));
  fragmentColor.rgb = vec3(1, 0, 0) * cos;
  fragmentColor.a = 1.0;
}
