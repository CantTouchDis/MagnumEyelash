#include <Corrade/Utility/Arguments.h>
#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/Shaders/GenericGL.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Version.h>
#include <Magnum/Shaders/VertexColorGL.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Magnum.h>
#include <Magnum/Shaders/FlatGL.h>


#include "EyelashTessellationShader.h"

using namespace Magnum;
using namespace Math::Literals;

class EyelashVisializer : public Platform::Application
{
public:
  explicit EyelashVisializer(const Arguments& arguments);
  void drawEvent() override;
private:
  GL::Buffer m_singleHairBuffer{NoCreate};
  GL::Mesh m_singleHair{NoCreate};
  GL::Mesh m_singleHairLine{NoCreate};
  EyelashTessellationShader m_coloredShader;
  Shaders::FlatGL3D m_flatColorShader;
  Matrix4 m_transformation, m_projection;
};


EyelashVisializer::EyelashVisializer(const Arguments& arguments) : Platform::Application{arguments, Configuration{}.setTitle("Eye Lash Visualizer")}
{
  GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
  GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
  GL::Renderer::setClearColor(0x0054AA_rgbf);

  // We require at least GL400 for tessellation
  MAGNUM_ASSERT_GL_VERSION_SUPPORTED(GL::Version::GL400);

  struct {
    Vector2 position;
    Color3 color;
  } hair[] =
  {
    {{0.0f, -1.0f}, 0xff0000_rgbf},
    {{-0.1f, -0.8f}, 0x00ff00_rgbf},
    {{0.3f, -0.8f}, 0x00ff00_rgbf},
    {{0.2f, -0.2f}, 0x00ff00_rgbf},
    {{0.6f,  0.2f}, 0x00ff00_rgbf},
    {{0.3f,  1.0f}, 0x0000ff_rgbf}
  };
  UnsignedByte indices[] = 
  {
    0, 0, 1, 2,
    0, 1, 2, 3,
    1, 2, 3, 4,
    2, 3, 4, 5,
    3, 4, 5, 5,
  };

  m_singleHairBuffer = GL::Buffer{};
  m_singleHairBuffer.setData(hair);
  GL::Buffer indicesBuffer;
  indicesBuffer.setData(indices);
  m_singleHair = GL::Mesh{};
  m_singleHair.setCount(Containers::arraySize(indices))
    .setPrimitive(GL::MeshPrimitive::Patches)
    .addVertexBuffer(m_singleHairBuffer, 0, Shaders::GenericGL2D::Position{}, Shaders::GenericGL2D::Color3{})
    .setIndexBuffer(std::move(indicesBuffer), 0, GL::MeshIndexType::UnsignedByte);

  m_singleHairLine = GL::Mesh{};
  m_singleHairLine.setCount(Containers::arraySize(hair))
    .setPrimitive(GL::MeshPrimitive::LineStrip)
    .addVertexBuffer(m_singleHairBuffer, 0, Shaders::GenericGL2D::Position{}, Shaders::GenericGL2D::Color3{});

  m_transformation =
    Matrix4::rotationX(0.0_degf)*Matrix4::rotationY(0.0_degf);
  m_projection =
    Matrix4::perspectiveProjection(
        35.0_degf, Vector2{windowSize()}.aspectRatio(), 0.01f, 100.0f)*
    Matrix4::translation(Vector3::zAxis(-5.0f));
}


void EyelashVisializer::drawEvent()
{
  GL::defaultFramebuffer.clear(GL::FramebufferClear::Color|GL::FramebufferClear::Depth);

  GL::Renderer::setPatchVertexCount(4);

  m_coloredShader.setColor(0xc7cf2f_rgbf).setTransformationMatrix(m_transformation).setProjectionMatrix(m_projection).draw(m_singleHair);

  m_flatColorShader.setColor(0xFF0000_rgbf).setTransformationProjectionMatrix(m_projection*m_transformation).draw(m_singleHairLine);

  swapBuffers();
  redraw();
  m_transformation = m_transformation * Matrix4::rotationY(0.1_degf);
}

MAGNUM_APPLICATION_MAIN(EyelashVisializer)
