#include "./EyelashShader.h"

#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>

using namespace Magnum;

EyelashShader::EyelashShader()
{
  MAGNUM_ASSERT_GL_VERSION_SUPPORTED(GL::Version::GL400);

  const Utility::Resource rs{"eyelash-data"};

  GL::Shader vs{GL::Version::GL400, GL::Shader::Type::Vertex};
  // GL::Shader tcs{GL::Version::GL400, GL::Shader::Type::TessellationControl};
  // GL::Shader tes{GL::Version::GL400, GL::Shader::Type::TessellationEvaluation};
  // GL::Shader gs{GL::Version::GL400, GL::Shader::Type::Geometry};
  GL::Shader fs{GL::Version::GL400, GL::Shader::Type::Fragment};

  vs.addSource(rs.get("shaders/TransformCoordinate.vert"));
  // tcs.addSource(rs.get("TexturedTriangleShader.vert"));
  // tes.addSource(rs.get("TexturedTriangleShader.vert"));
  // gs.addSource(rs.get("shaders/EyelashShader.geom"));
  fs.addSource(rs.get("shaders/EyelashShader.frag"));

  CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({vs, fs}));

  attachShaders({vs, fs});

  CORRADE_INTERNAL_ASSERT_OUTPUT(link());

  m_colorUniform = uniformLocation("color");
  m_transformationMatrixUniform = uniformLocation("transformationMatrix");
  m_projectionMatrixUniform = uniformLocation("projectionMatrix");
}
