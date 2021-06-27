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
#include <Magnum/Shaders/PhongGL.h>

#include <Magnum/ImGuiIntegration/Context.hpp>

// Scene graph
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Scene.h>
// loader
#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/PhongMaterialData.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/Trade/SceneData.h>
#include <Magnum/Trade/MeshObjectData3D.h>
#include <Magnum/MeshTools/Compile.h>

#include "EyelashTessellationShader.h"

using namespace Magnum;
using namespace Math::Literals;

typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;
typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;

class EyelashVisualizer : public Platform::Application
{
public:
  explicit EyelashVisualizer(const Arguments& arguments);
  void drawEvent() override;
private:
  // Keyboard
  void keyPressEvent(KeyEvent& event) override;
  void keyReleaseEvent(KeyEvent& event) override;
  void textInputEvent(TextInputEvent& event) override;
  // Mouse
  void mousePressEvent(MouseEvent& event) override;
  void mouseReleaseEvent(MouseEvent& event) override;
  void mouseMoveEvent(MouseMoveEvent& event) override;
  void viewportEvent(ViewportEvent& event) override;

  void loadScene(const std::string& fileName);
  void addObject(Trade::AbstractImporter& importer, Containers::ArrayView<const Containers::Optional<Trade::PhongMaterialData>> materials, Object3D& parent, UnsignedInt i);

  // Imgui
  ImGuiIntegration::Context m_imgui{NoCreate};


  std::vector<std::string> m_sceneNames;
  size_t m_currentScene = 0;

  // Single Hair scene.
  GL::Buffer m_singleHairBuffer{NoCreate};
  GL::Mesh m_singleHair{NoCreate};
  GL::Mesh m_singleHairLine{NoCreate};

  std::vector<EyelashTessellationShader> m_coloredShaders;
  std::vector<std::string> m_shaderNames;
  size_t m_currentShader = 0;

  Shaders::PhongGL m_coloredShader;
  Shaders::FlatGL3D m_flatColorShader;
  Matrix4 m_transformation, m_projection;

  // Camera
  Vector2i m_previousMousePosition;  // used to rotate the view.
  Matrix4 m_view;
  Vector3 m_cameraPosition{0.0f, 0.0f, -5.0f};
  Vector2 m_cameraRotation;

  // Scene
  Containers::Array<Containers::Optional<GL::Mesh>> m_meshes;
  Scene3D m_scene;
  SceneGraph::Camera3D* m_camera = nullptr;
  Object3D m_cameraObject,
           m_manipulator;
  SceneGraph::DrawableGroup3D m_drawables;

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
  Utility::Arguments args;
  args.addOption("file").setHelp("file", "file to load")
        .parse(arguments.argc, arguments.argv);

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

  m_sceneNames.push_back("Single Hair");
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
  m_singleHairLine.setCount(Containers::arraySize(hair) - 2)
    .setPrimitive(GL::MeshPrimitive::LineStrip)
    // 4 bytes are unused, cause they define the hair width.
    // start at the second point (first is an helper control point)
    .addVertexBuffer(m_singleHairBuffer, 4 * 4, Shaders::FlatGL3D::Position{}, 4);

  m_transformation =
    Matrix4::rotationX(0.0_degf)*Matrix4::rotationY(0.0_degf);
  m_projection =
    Matrix4::perspectiveProjection(
        35.0_degf, Vector2{windowSize()}.aspectRatio(), 0.001f, 100.0f);

  // setMinimalLoopPeriod(16);
  
  
  m_coloredShaders.emplace_back(EyelashTessellationShader::DEFAULT);
  m_shaderNames.push_back("default");
  m_coloredShaders.emplace_back(EyelashTessellationShader::NORMAL);
  m_shaderNames.push_back("normal");
  m_coloredShaders.emplace_back(EyelashTessellationShader::WIREFRAME);
  m_shaderNames.push_back("wireframe");
  m_coloredShaders.emplace_back(EyelashTessellationShader::NORMAL | EyelashTessellationShader::WIREFRAME);
  m_shaderNames.push_back("normal_wireframe");


  // no extra scene given.
  if (args.value("file").size() == 0)
    return;
  // load extra scene.
  loadScene(args.value("file"));
}

// phong drawable
class ColoredDrawable: public SceneGraph::Drawable3D
{
public:
  explicit ColoredDrawable(Object3D& object, Shaders::PhongGL& shader, GL::Mesh& mesh, const Color4& color, SceneGraph::DrawableGroup3D& group): SceneGraph::Drawable3D{object, &group}, m_shader(shader), m_mesh(mesh), m_color{color} {}

private:
  void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;

  Shaders::PhongGL& m_shader;
  GL::Mesh& m_mesh;
  Color4 m_color;
};


void ColoredDrawable::draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
  m_shader
    .setDiffuseColor(m_color)
     .setLightPositions({
         {camera.cameraMatrix().transformPoint({-3.0f, 10.0f, 10.0f}), 0.0f}
     })
    .setTransformationMatrix(transformationMatrix)
    .setNormalMatrix(transformationMatrix.normalMatrix())
    .setProjectionMatrix(camera.projectionMatrix())
    .draw(m_mesh);
}


void EyelashVisualizer::loadScene(const std::string& fileName)
{
  /* Load a scene importer plugin */
  PluginManager::Manager<Trade::AbstractImporter> manager;
  Containers::Pointer<Trade::AbstractImporter> importer = manager.loadAndInstantiate("AnySceneImporter");
  if(!importer)
  {
    Warning{} << "Could not create importer, skipping other scenes.";
    return;
  }

  if(!importer->openFile(fileName))
  {
    Warning{} << "Could not open scene: " << fileName.c_str();
    return;
  }

  m_manipulator.setParent(&m_scene);
  m_cameraObject.setParent(&m_scene)
        .translate(Vector3::zAxis(5.0f));
  (*(m_camera = new SceneGraph::Camera3D{m_cameraObject}))
      .setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
      .setProjectionMatrix(Matrix4::perspectiveProjection(35.0_degf, 1.0f, 0.01f, 1000.0f))
      .setViewport(GL::defaultFramebuffer.viewport().size());
  // load the scene.
  Containers::Array<Containers::Optional<Trade::PhongMaterialData>> materials{importer->materialCount()};
  for(UnsignedInt i = 0; i != importer->materialCount(); ++i)
  {
    Debug{} << "Importing material" << i << importer->materialName(i).c_str();

    Containers::Optional<Trade::MaterialData> materialData = importer->material(i);
    if(!materialData || !(materialData->types() & Trade::MaterialType::Phong)) {
        Warning{} << "Cannot load material, skipping";
        continue;
    }

    materials[i] = std::move(static_cast<Trade::PhongMaterialData&>(*materialData));
  }
  /* Load all meshes. Meshes that fail to load will be NullOpt. */
  m_meshes = Containers::Array<Containers::Optional<GL::Mesh>>{importer->meshCount()};
  for (UnsignedInt i = 0; i != importer->meshCount(); ++i)
  {
    Debug{} << "Importing mesh" << i << importer->meshName(i).c_str();

    Containers::Optional<Trade::MeshData> meshData = importer->mesh(i);
    auto isMatchingPrimitive = meshData->primitive() == MeshPrimitive::Triangles || meshData->primitive() == MeshPrimitive::Lines;
    if (!meshData || (meshData->primitive() == MeshPrimitive::Triangles && !meshData->hasAttribute(Trade::MeshAttribute::Normal)) || !isMatchingPrimitive) {
        Warning{} << meshData->primitive();
        Warning{} << "Cannot load the mesh, skipping";
        continue;
    }

    /* Compile the mesh */
    m_meshes[i] = MeshTools::compile(*meshData);
  }
  // add to scenes
  if (importer->defaultScene() != -1) {
    Debug{} << "Adding default scene" << importer->sceneName(importer->defaultScene()).c_str();

    Containers::Optional<Trade::SceneData> sceneData = importer->scene(importer->defaultScene());
    if(!sceneData) {
        Error{} << "Cannot load scene, exiting";
        return;
    }

    /* Recursively add all children */
    for(UnsignedInt objectId: sceneData->children3D())
        addObject(*importer, materials, m_manipulator, objectId);

    /* The format has no scene support, display just the first loaded mesh with
       a default material and be done with it */
  } else if(!m_meshes.empty() && m_meshes[0])
        new ColoredDrawable{m_manipulator, m_coloredShader, *m_meshes[0], 0xffffff_rgbf, m_drawables};


  m_sceneNames.push_back(fileName);
}

void EyelashVisualizer::addObject(Trade::AbstractImporter& importer, Containers::ArrayView<const Containers::Optional<Trade::PhongMaterialData>> materials, Object3D& parent, UnsignedInt i)
{
    Debug{} << "Importing object" << i << importer.object3DName(i).c_str();
    Containers::Pointer<Trade::ObjectData3D> objectData = importer.object3D(i);
    if(!objectData)
    {
      Error{} << "Cannot import object, skipping";
      return;
    }

    /* Add the object to the scene and set its transformation */
    auto* object = new Object3D{&parent};
    object->setTransformation(objectData->transformation());

    /* Add a drawable if the object has a mesh and the mesh is loaded */
    if(objectData->instanceType() == Trade::ObjectInstanceType3D::Mesh && objectData->instance() != -1 && m_meshes[objectData->instance()]) {
      const Int materialId = static_cast<Trade::MeshObjectData3D*>(objectData.get())->material();

      /* Material not available / not loaded, use a default material */
      if (materialId == -1 || !materials[materialId])
      {
        new ColoredDrawable{*object, m_coloredShader, *m_meshes[objectData->instance()], 0xffffff_rgbf, m_drawables};
      }
      else {
        new ColoredDrawable{*object, m_coloredShader, *m_meshes[objectData->instance()], materials[materialId]->diffuseColor(), m_drawables};
      }
    }

    /* Recursively add children */
    for (std::size_t id: objectData->children())
      addObject(importer, materials, *object, id);
}


void EyelashVisualizer::viewportEvent(ViewportEvent& event)
{
  GL::defaultFramebuffer.setViewport({{}, event.framebufferSize()});

  m_camera->setViewport(event.windowSize());

  m_imgui.relayout(Vector2{event.windowSize()}/event.dpiScaling(),
      event.windowSize(), event.framebufferSize());
}



void EyelashVisualizer::drawEvent()
{
  GL::defaultFramebuffer.clear(GL::FramebufferClear::Color|GL::FramebufferClear::Depth);


  // uniforms
  static Vector3 wireFrameColor = 0xFF0000_rgbf;
  static Vector3 hairColor = 0xc7cf2f_rgbf;
  static int cylinderSegmentCount = 30;
  static int desiredTessellarion = 30;

  m_imgui.newFrame();
  /* Enable text input, if needed */
  if(ImGui::GetIO().WantTextInput && !isTextInputActive())
    startTextInput();
  else if(!ImGui::GetIO().WantTextInput && isTextInputActive())
    stopTextInput();
  {
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
        1000.0 / Double(ImGui::GetIO().Framerate), Double(ImGui::GetIO().Framerate));
    ImGui::Text("Scene:");
    ImGui::SameLine();
    if (ImGui::BeginCombo("##Scene", m_sceneNames[m_currentScene].c_str()))
    {
      for (size_t i = 0; i < m_sceneNames.size(); i++)
      {
        bool isSelected = m_currentScene == i;
        if (ImGui::Selectable(m_sceneNames[i].c_str(), isSelected))
          m_currentScene = i;
        if (isSelected)
          ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
    }
    if (ImGui::TreeNode("Shader"))
    {
      ImGui::Text("Type:");
      ImGui::SameLine();
      if (ImGui::BeginCombo("##Shader", m_shaderNames[m_currentShader].c_str()))
      {
        for (size_t i = 0; i < m_shaderNames.size(); ++i)
        {
          bool isSelected = i == m_currentShader;
          if (ImGui::Selectable(m_shaderNames[i].c_str(), isSelected))
            m_currentShader = i;
          if (isSelected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }

      ImGui::Text("HairColor:");
      ImGui::ColorEdit3("##hairColor", hairColor.data());
      ImGui::Text("WireFrameColor:");
      ImGui::ColorEdit3("##wireframeColor", wireFrameColor.data());
      ImGui::Text("GeometrySegments:");
      ImGui::SliderInt("##cylinderSegments", &cylinderSegmentCount, 4, 32);
      ImGui::Text("TessellationSegments:");
      ImGui::SliderInt("##desiredTessellarion", &desiredTessellarion, 1, 32);
//       if(ImGui::Button("Test Window"))
//           _showDemoWindow ^= true;
//       if(ImGui::Button("Another Window"))
//           _showAnotherWindow ^= true;
      ImGui::TreePop();
    }
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

  if (m_currentScene == 0)
  {
    GL::Renderer::setPatchVertexCount(4);

    m_coloredShaders[m_currentShader]
      .setColor(hairColor)
      .setWireFrameColor(wireFrameColor)
      .setCylinderSegmentCount(cylinderSegmentCount)
      .setDesirededgeTessellation(desiredTessellarion)
      .setTransformationMatrix(m_view*m_transformation).setProjectionMatrix(m_projection).draw(m_singleHair);

    m_flatColorShader.setColor(0xFF0000_rgbf).setTransformationProjectionMatrix(m_projection*m_view*m_transformation).draw(m_singleHairLine);
  }
  else
  {
    m_camera->draw(m_drawables);
  }

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

void EyelashVisualizer::textInputEvent(TextInputEvent& event)
{
  if (m_imgui.handleTextInputEvent(event)) return;
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
