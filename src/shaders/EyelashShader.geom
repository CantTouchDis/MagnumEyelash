layout(lines) in;

in vec4 tangent_tes[];
in float width_tes[];


uniform mat4 transformationMatrix;
uniform mat4 projectionMatrix;
uniform int cylinderSegmentCount = 30;


// if cylinderSegmentCount is not a uniform we can go higher than 92. 
// have to read into why changing it to an uniform reduces the number of vertices we can emit. 
layout(triangle_strip, max_vertices = 92) out;

out vec4 normal;
#if defined(WIREFRAME)
out vec3 bary;
#endif

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

void main()
{
  vec3 circle_0_x = compute_perpendicular_to_direction(tangent_tes[0].xyz);
  vec3 circle_1_x = compute_perpendicular_to_direction(tangent_tes[1].xyz);

  vec3 circle_0_y = normalize(cross(tangent_tes[0].xyz, circle_0_x));
  vec3 circle_1_y = normalize(cross(tangent_tes[1].xyz, circle_1_x));

  int segment_count = max(4, min(cylinderSegmentCount, 40));

  for (int i = 0; i <= segment_count; i++)
  {
    float rad = -i / float(segment_count) * 6.28318530717958f;
    float u = cos(rad);
    float v = sin(rad);
    vec4 normal_0 = vec4(u * circle_0_x + v * circle_0_y, 0);
    vec4 normal_1 = vec4(u * circle_1_x + v * circle_1_y, 0);
#if defined(WIREFRAME)
    int first = (i * 2) % 3;
    int second = (i * 2 + 1) % 3;
    vec3 baryCoord = vec3(0, 0, 0);
    baryCoord[first] = 1.0f;
    bary = baryCoord;
#endif
    gl_Position = projectionMatrix * transformationMatrix * (gl_in[0].gl_Position + width_tes[0] * normal_0);
    normal = normal_0;
    EmitVertex();
#if defined(WIREFRAME)
    baryCoord = vec3(0);
    baryCoord[second] = 1.0f;
    bary = baryCoord;
#endif
    gl_Position = projectionMatrix * transformationMatrix * (gl_in[1].gl_Position + width_tes[1] * normal_1);
    normal = normal_1;
    EmitVertex();
  }
  EndPrimitive();
}
