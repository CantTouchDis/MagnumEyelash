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
  
  explicit EyelashTessellationShader();
  
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

private:
  Magnum::Int m_colorUniform,
    m_transformationMatrixUniform,
    m_projectionMatrixUniform;
};
