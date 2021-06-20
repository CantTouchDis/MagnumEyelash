
uniform vec3 hairColor = vec3(1.0, 1.0, 1.0);

in vec4 normal;
out vec4 fragmentColor;


const float refraction = 1.55f;

void main()
{
#ifdef shade_normal
  fragmentColor.rgb = normal.xyz * 0.5f + 0.5f;
#else
  float cos = max(0, dot(normal.xyz, normalize(vec3(0, 1, 1))));
  fragmentColor.rgb = hairColor * cos;
#endif
  fragmentColor.a = 1.0;
}
