layout(lines) in;

uniform mat4 transformationMatrix;
uniform mat4 projectionMatrix;

in vec4 tangent[];

out vec3 normal;

layout(triangle_strip, max_vertices = 64) out;

// computes a vector perpendicular to the given direction
vec3 compute_perpendicular_to_direction(vec3 direction)
{
  vec3 dir_norm = normalize(direction);
  vec3 result = cross(dir_norm, vec3(0, 0, 1));
  if (dot(result, result) == 0.0)
  {
    result = cross(dir_norm, vec3(0, 1, 0));
  }

  return result;
}

void main() {
  
  vec3 circle_0_x = compute_perpendicular_to_direction(tangent[0].xyz);
  vec3 circle_1_x = compute_perpendicular_to_direction(tangent[1].xyz);
  vec3 circle_0_y = normalize(cross(tangent[0].xyz, circle_0_x));
  vec3 circle_1_y = normalize(cross(tangent[1].xyz, circle_1_x));

  int segment_count = 8 /*max(4, segments)*/;

  for (int i = 0; i <= segment_count; i++)
  {
    float rad = -i / float(segment_count) * 6.28318530717958f;
    float u = cos(rad);
    float v = sin(rad);
    vec3 normal_0 = u * circle_0_x + v * circle_0_y;
    vec3 normal_1 = u * circle_1_x + v * circle_1_y;
    gl_Position = projectionMatrix * transformationMatrix * (gl_in[0].gl_Position + 0.02f * vec4(normal_0, 0));
    normal = normal_0;
    EmitVertex();
    gl_Position = projectionMatrix * transformationMatrix * (gl_in[1].gl_Position + 0.02f * vec4(normal_1, 0));
    normal = normal_1;
    EmitVertex();
  }
  EndPrimitive();
}
