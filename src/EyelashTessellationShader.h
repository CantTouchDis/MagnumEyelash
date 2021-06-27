#pragma once

#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/GL/Attribute.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Matrix4.h>

class EyelashTessellationShader: public Magnum::GL::AbstractShaderProgram
{
public:
  typedef Magnum::GL::Attribute<0, Magnum::Vector3> Position;
  typedef Magnum::GL::Attribute<1, Magnum::Float> HairWidth;
  enum ShaderType
  {
    DEFAULT = 0,
    WIREFRAME = 1,
    FLAT = 2,
    NORMAL = 4,
  };
  
  explicit EyelashTessellationShader(ShaderType type = DEFAULT);
  
  EyelashTessellationShader& setTransformationMatrix(const Magnum::Matrix4& matrix) {
      setUniform(m_transformationMatrixUniform, matrix);
      return *this;
  }

  EyelashTessellationShader& setProjectionMatrix(const Magnum::Matrix4& matrix) {
      setUniform(m_projectionMatrixUniform, matrix);
      return *this;
  }

  EyelashTessellationShader& setColor(const Magnum::Color3& color) {
      setUniform(m_colorUniform, color);
      return *this;
  }

  EyelashTessellationShader& setWireFrameColor(const Magnum::Color3& color) {
      setUniform(m_wireFrameColorUniform, color);
      return *this;
  }

  EyelashTessellationShader& setCylinderSegmentCount(int numSegments) {
      setUniform(m_cylinderSegmentCountUniform, numSegments);
      return *this;
  }

  EyelashTessellationShader& setDesirededgeTessellation(int split) {
      setUniform(m_desiredEdgeTessellationUniform, static_cast<float>(split));
      return *this;
  }
  EyelashTessellationShader& setLightDirection(const Magnum::Vector3& dir) {
      setUniform(m_lightPosUniform, dir);
      return *this;
  }

private:
  Magnum::Int m_colorUniform,
    m_transformationMatrixUniform,
    m_projectionMatrixUniform,
    m_wireFrameColorUniform,
    m_cylinderSegmentCountUniform,
    m_lightPosUniform,
    m_desiredEdgeTessellationUniform;
};

inline EyelashTessellationShader::ShaderType operator|(EyelashTessellationShader::ShaderType a, EyelashTessellationShader::ShaderType b)
{
  return static_cast<EyelashTessellationShader::ShaderType>(static_cast<int>(a) | static_cast<int>(b));
}
