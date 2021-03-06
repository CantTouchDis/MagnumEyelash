#include "./EyelashTessellationShader.h"

#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>

using namespace Magnum;

EyelashTessellationShader::EyelashTessellationShader(ShaderType type)
{
  MAGNUM_ASSERT_GL_VERSION_SUPPORTED(GL::Version::GL400);

  const Utility::Resource rs{"eyelash-data"};

  GL::Shader vs{GL::Version::GL400, GL::Shader::Type::Vertex};
  GL::Shader tcs{GL::Version::GL400, GL::Shader::Type::TessellationControl};
  GL::Shader tes{GL::Version::GL400, GL::Shader::Type::TessellationEvaluation};
  GL::Shader gs{GL::Version::GL400, GL::Shader::Type::Geometry};
  GL::Shader fs{GL::Version::GL400, GL::Shader::Type::Fragment};

  // add defines depending on the type
  if (type & WIREFRAME)
  {
    gs.addSource("#define WIREFRAME");
    fs.addSource("#define WIREFRAME");
  }
  if (type & NORMAL)
  {
    fs.addSource("#define SHADE_NORMAL\n");
  }

  vs.addSource(rs.get("shaders/EyelashShader.vert"));
  tcs.addSource(rs.get("shaders/EyelashShader.tcs"));
  tes.addSource(rs.get("shaders/EyelashShader.tes"));
  gs.addSource(rs.get("shaders/EyelashShader.geom"));
  fs.addSource(rs.get("shaders/EyelashShader.frag"));

  CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({vs, tcs, tes, gs, fs}));

  attachShaders({vs, tcs, tes, gs, fs});

  CORRADE_INTERNAL_ASSERT_OUTPUT(link());

  if (!(type & NORMAL))
  {
    m_colorUniform = uniformLocation("hairColor");
    m_lightPosUniform = uniformLocation("lightDir");
  }
  if (type & WIREFRAME)
  {
    m_wireFrameColorUniform = uniformLocation("wireFrameColor");
  }
  m_desiredEdgeTessellationUniform = uniformLocation("desiredEdges");
  m_cylinderSegmentCountUniform = uniformLocation("cylinderSegmentCount");
  m_transformationMatrixUniform = uniformLocation("transformationMatrix");
  m_projectionMatrixUniform = uniformLocation("projectionMatrix");
}
