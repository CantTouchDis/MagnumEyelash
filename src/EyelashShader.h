#pragma once

#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Matrix4.h>

class EyelashShader: public Magnum::GL::AbstractShaderProgram
{
public:
  // typedef GL::Attribute<0, Vector2> Position;
  // typedef GL::Attribute<1, Vector2> TextureCoordinates;
  
  explicit EyelashShader();
  
  EyelashShader& setTransformationMatrix(const Magnum::Matrix4& matrix) {
      setUniform(m_transformationMatrixUniform, matrix);
      return *this;
  }

  EyelashShader& setProjectionMatrix(const Magnum::Matrix4& matrix) {
      setUniform(m_projectionMatrixUniform, matrix);
      return *this;
  }

  EyelashShader& setColor(const Magnum::Color3& color) {
      setUniform(m_colorUniform, color);
      return *this;
  }

private:
  Magnum::Int m_colorUniform,
    m_transformationMatrixUniform,
    m_projectionMatrixUniform;
};
