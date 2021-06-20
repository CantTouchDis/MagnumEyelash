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
  // Keyboard
  void keyPressEvent(KeyEvent& event) override;
  void keyReleaseEvent(KeyEvent& event) override;
  // Mouse
  void mousePressEvent(MouseEvent& event) override;
  void mouseReleaseEvent(MouseEvent& event) override;
  void mouseMoveEvent(MouseMoveEvent& event) override;
  GL::Buffer m_singleHairBuffer{NoCreate};
  GL::Mesh m_singleHair{NoCreate};
  GL::Mesh m_singleHairLine{NoCreate};
  EyelashTessellationShader m_coloredShader;
  Shaders::FlatGL3D m_flatColorShader;
  Matrix4 m_transformation, m_projection;

  // Camera
  Vector2i m_previousMousePosition;  // used to rotate the view.
  Matrix4 m_view;
  Vector3 m_cameraPosition{0.0f, 0.0f, -5.0f};
  Vector2 m_cameraRotation;

  struct
  {
    bool forward;
    bool backward;
    bool left;
    bool right;
  } m_input;
};


EyelashVisializer::EyelashVisializer(const Arguments& arguments) : Platform::Application{arguments, Configuration{}.setTitle("Eye Lash Visualizer")}
{
  GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
  GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
  GL::Renderer::setClearColor(0x0054AA_rgbf);

  // We require at least GL400 for tessellation
  MAGNUM_ASSERT_GL_VERSION_SUPPORTED(GL::Version::GL400);

  struct {
    Vector3 position;
    float hairWidth;
  } hair[] =
  {
    {{0.0f, -1.0f, 0.0f},  0.02f},
    {{-0.1f, -0.8f, 0.0f}, 0.015f},
    {{0.3f, -0.8f, 0.2f},  0.015f},
    {{0.2f, -0.2f, -0.2f}, 0.015f},
    {{0.6f,  0.2f, 0.0f},  0.01f},
    {{0.3f,  1.0f, 0.0f},  0.0f}
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
    .addVertexBuffer(m_singleHairBuffer, 0, EyelashTessellationShader::Position{}, EyelashTessellationShader::HairWidth{})
    .setIndexBuffer(std::move(indicesBuffer), 0, GL::MeshIndexType::UnsignedByte);

  m_singleHairLine = GL::Mesh{};
  m_singleHairLine.setCount(Containers::arraySize(hair))
    .setPrimitive(GL::MeshPrimitive::LineStrip)
    // 4 bytes are unused, cause they define the hair width.
    .addVertexBuffer(m_singleHairBuffer, 0, Shaders::FlatGL3D::Position{}, 4);

  m_transformation =
    Matrix4::rotationX(0.0_degf)*Matrix4::rotationY(0.0_degf);
  m_projection =
    Matrix4::perspectiveProjection(
        35.0_degf, Vector2{windowSize()}.aspectRatio(), 0.01f, 100.0f);
}


void EyelashVisializer::drawEvent()
{
  GL::defaultFramebuffer.clear(GL::FramebufferClear::Color|GL::FramebufferClear::Depth);

  Vector3 cameraDirection;
  // forward/backward movement:
  cameraDirection = (m_input.forward - m_input.backward) * m_view.inverted().backward() * 0.01f;
  // left/right movement:
  cameraDirection += (m_input.right - m_input.left) * Math::cross(m_view.inverted().backward(), {0.0f, 1.0f, 0.0}) * 0.01f;
  m_cameraPosition += cameraDirection;
  m_view =
    Matrix4::rotationX(Rad{m_cameraRotation.y()}) *
    Matrix4::rotationY(Rad{m_cameraRotation.x()}) *
    Matrix4::translation(m_cameraPosition);

  GL::Renderer::setPatchVertexCount(4);

  m_coloredShader.setColor(0xc7cf2f_rgbf).setTransformationMatrix(m_view*m_transformation).setProjectionMatrix(m_projection).draw(m_singleHair);

  m_flatColorShader.setColor(0xFF0000_rgbf).setTransformationProjectionMatrix(m_projection*m_view*m_transformation).draw(m_singleHairLine);

  swapBuffers();
  redraw();
}


void EyelashVisializer::keyPressEvent(KeyEvent& event)
{
  switch (event.key())
  {
    case KeyEvent::Key::W:
      m_input.forward = true;
      break;
    case KeyEvent::Key::S:
      m_input.backward = true;
      break;
    case KeyEvent::Key::A:
      m_input.left = true;
      break;
    case KeyEvent::Key::D:
      m_input.right = true;
      break;
    default:
      break;
  }
}
void EyelashVisializer::keyReleaseEvent(KeyEvent& event)
{
  switch (event.key())
  {
    case KeyEvent::Key::W:
      m_input.forward = false;
      break;
    case KeyEvent::Key::S:
      m_input.backward = false;
      break;
    case KeyEvent::Key::A:
      m_input.left = false;
      break;
    case KeyEvent::Key::D:
      m_input.right = false;
      break;
      // TODO: Shader reload in debug mode.
    default:
      break;
  }
}

void EyelashVisializer::mousePressEvent(MouseEvent& event)
{
  if((event.button() == MouseEvent::Button::Left))
  {
    m_previousMousePosition = event.position();
  }
}
void EyelashVisializer::mouseReleaseEvent(MouseEvent& /*event*/)
{
  redraw();
}
void EyelashVisializer::mouseMoveEvent(MouseMoveEvent& event)
{
  if ((event.buttons() & MouseMoveEvent::Button::Left))
  {
    const Vector2 delta = 3.0f *
        Vector2{event.position() - m_previousMousePosition} / Vector2{GL::defaultFramebuffer.viewport().size()};
    m_cameraRotation += delta;

    m_previousMousePosition = event.position();
    redraw();
  }
}

MAGNUM_APPLICATION_MAIN(EyelashVisializer)
