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

#include <Magnum/ImGuiIntegration/Context.hpp>

#include "EyelashTessellationShader.h"

using namespace Magnum;
using namespace Math::Literals;

class EyelashVisualizer : public Platform::Application
{
public:
  explicit EyelashVisualizer(const Arguments& arguments);
  void drawEvent() override;
private:
  // Keyboard
  void keyPressEvent(KeyEvent& event) override;
  void keyReleaseEvent(KeyEvent& event) override;
  // Mouse
  void mousePressEvent(MouseEvent& event) override;
  void mouseReleaseEvent(MouseEvent& event) override;
  void mouseMoveEvent(MouseMoveEvent& event) override;
  void viewportEvent(ViewportEvent& event) override;

  // Imgui
  ImGuiIntegration::Context m_imgui{NoCreate};

  GL::Buffer m_singleHairBuffer{NoCreate};
  GL::Mesh m_singleHair{NoCreate};
  GL::Mesh m_singleHairLine{NoCreate};
  EyelashTessellationShader m_coloredShader;
  Shaders::FlatGL3D m_flatColorShader;
  Matrix4 m_transformation, m_projection;

  // Uniforms/Toggles
  Color3 m_hairColor = 0xc7cf2f_rgbf;

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
  } m_input = {};
};


EyelashVisualizer::EyelashVisualizer(const Arguments& arguments) : Platform::Application{arguments,
  Configuration{}.setTitle("Eye Lash Visualizer").setWindowFlags(Configuration::WindowFlag::Resizable)}
{

  m_imgui = ImGuiIntegration::Context(Vector2{windowSize()}/dpiScaling(),
        windowSize(), framebufferSize());

  GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add,
  GL::Renderer::BlendEquation::Add);
  GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
  GL::Renderer::BlendFunction::OneMinusSourceAlpha);

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
    {{0.1f, -1.2f, 0.0f},  0.00f},  // control point
    {{0.0f, -1.0f, 0.0f},  0.02f},
    {{-0.1f, -0.8f, 0.0f}, 0.015f},
    {{0.3f, -0.8f, 0.2f},  0.015f},
    {{0.2f, -0.2f, -0.2f}, 0.015f},
    {{0.6f,  0.2f, 0.0f},  0.01f},
    {{0.3f,  1.0f, 0.0f},  0.0f},
    {{0.0f,  1.8f, 0.0f},  0.0f}  // control point
  };
  UnsignedByte indices[] = 
  {
    0, 1, 2, 3,
    1, 2, 3, 4,
    2, 3, 4, 5,
    3, 4, 5, 6,
    4, 5, 6, 7
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

  // setMinimalLoopPeriod(16);
}


void EyelashVisualizer::viewportEvent(ViewportEvent& event)
{
  GL::defaultFramebuffer.setViewport({{}, event.framebufferSize()});

  m_imgui.relayout(Vector2{event.windowSize()}/event.dpiScaling(),
      event.windowSize(), event.framebufferSize());
}



void EyelashVisualizer::drawEvent()
{
  GL::defaultFramebuffer.clear(GL::FramebufferClear::Color|GL::FramebufferClear::Depth);

      /* Enable text input, if needed */
  if(ImGui::GetIO().WantTextInput && !isTextInputActive())
      startTextInput();
  else if(!ImGui::GetIO().WantTextInput && isTextInputActive())
      stopTextInput();
  float _floatValue;
  m_imgui.newFrame();
  {
    ImGui::Text("Hello, world!");
    ImGui::SliderFloat("Float", &_floatValue, 0.0f, 1.0f);
    ImGui::ColorEdit3("Clear Color", m_hairColor.data());
//     if(ImGui::Button("Test Window"))
//         _showDemoWindow ^= true;
//     if(ImGui::Button("Another Window"))
//         _showAnotherWindow ^= true;
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
        1000.0/Double(ImGui::GetIO().Framerate), Double(ImGui::GetIO().Framerate));
  }

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

  m_coloredShader.setColor(m_hairColor).setTransformationMatrix(m_view*m_transformation).setProjectionMatrix(m_projection).draw(m_singleHair);

  m_flatColorShader.setColor(0xFF0000_rgbf).setTransformationProjectionMatrix(m_projection*m_view*m_transformation).draw(m_singleHairLine);


  /* Set appropriate states. If you only draw ImGui, it is sufficient to
       just enable blending and scissor test in the constructor. */
    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);
    GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);
    GL::Renderer::disable(GL::Renderer::Feature::DepthTest);

    m_imgui.drawFrame();

    /* Reset state. Only needed if you want to draw something else with
       different state after. */
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
    GL::Renderer::disable(GL::Renderer::Feature::ScissorTest);
    GL::Renderer::disable(GL::Renderer::Feature::Blending);


  swapBuffers();
  redraw();
}


void EyelashVisualizer::keyPressEvent(KeyEvent& event)
{
  if (m_imgui.handleKeyPressEvent(event)) return;
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
void EyelashVisualizer::keyReleaseEvent(KeyEvent& event)
{
  if (m_imgui.handleKeyReleaseEvent(event)) return;
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

void EyelashVisualizer::mousePressEvent(MouseEvent& event)
{
  if (m_imgui.handleMousePressEvent(event)) return;
  if((event.button() == MouseEvent::Button::Left))
  {
    m_previousMousePosition = event.position();
  }
}
void EyelashVisualizer::mouseReleaseEvent(MouseEvent& event)
{
  if (m_imgui.handleMouseReleaseEvent(event)) return;
  redraw();
}
void EyelashVisualizer::mouseMoveEvent(MouseMoveEvent& event)
{
  if (m_imgui.handleMouseMoveEvent(event)) return;
  if ((event.buttons() & MouseMoveEvent::Button::Left))
  {
    const Vector2 delta = 3.0f *
        Vector2{event.position() - m_previousMousePosition} / Vector2{GL::defaultFramebuffer.viewport().size()};
    m_cameraRotation += delta;

    m_previousMousePosition = event.position();
    redraw();
  }
}

MAGNUM_APPLICATION_MAIN(EyelashVisualizer)
