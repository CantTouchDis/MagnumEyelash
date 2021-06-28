in vec4 normal;
#if defined(WIREFRAME)
noperspective in vec3 bary;
#endif

uniform vec3 lightDir = vec3(0.0, 1.0, 1.0);
uniform vec3 hairColor = vec3(1.0, 1.0, 1.0);
#if defined(WIREFRAME)
uniform vec3 wireFrameColor = vec3(1.0f, 0.0f, 0.0f);
#endif

out vec4 fragmentColor;

// Hair refraction
const float refraction = 1.55f;


#if defined(WIREFRAME)
float edgeFactor() {
  vec3 d = fwidth(bary);
  vec3 f = step(d * 1, bary);
  return min(min(f.x, f.y), f.z);
}
#endif

void main()
{
#if defined(SHADE_NORMAL)
  fragmentColor.rgb = normal.xyz * 0.5f + 0.5f;
#else
  float cos = max(0, dot(normal.xyz, normalize(lightDir)));
  fragmentColor.rgb = hairColor * cos;
#endif
#if defined(WIREFRAME)
  // in case we should display a wireframe, we need to set the color on the edges where it is needed.
  float factor = edgeFactor();
  fragmentColor.rgb = fragmentColor.rgb * (factor) + (1.0f-factor) * wireFrameColor;
#endif
  fragmentColor.a = 1.0;
}
