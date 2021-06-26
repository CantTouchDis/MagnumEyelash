find_package(Corrade REQUIRED Main)
find_package(Magnum REQUIRED
    GL
    MeshTools
    Shaders
    SceneGraph
    Trade
    Sdl2Application)
find_package(MagnumIntegration REQUIRED ImGui)

set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)

corrade_add_resource(Eyelash_RESOURCES resources.conf)

#add_executable(MyApplication
#  MyApplication.cpp
#  EyelashShader.cpp
#  EyelashTessellationShader.cpp
#  ${Eyelash_RESOURCES})
#
#target_link_libraries(MyApplication PRIVATE
#    Corrade::Main
#    Magnum::Application
#    Magnum::GL
#    Magnum::Magnum
#    Magnum::MeshTools
#    Magnum::SceneGraph
#    Magnum::Shaders
#    Magnum::Trade)
#
#add_dependencies(MyApplication
#    Magnum::AnyImageImporter
#    Magnum::AnySceneImporter
#    Magnum::ObjImporter
#    MagnumPlugins::StbImageImporter
#    MagnumPlugins::TinyGltfImporter)

add_executable(EyelashVisualizer
  EyelashVisualizer.cpp
  EyelashShader.cpp
  EyelashTessellationShader.cpp
  ${Eyelash_RESOURCES})

target_link_libraries(EyelashVisualizer PRIVATE
    Corrade::Main
    Magnum::Application
    Magnum::GL
    Magnum::Magnum
    #    Magnum::MeshTools
    #    Magnum::SceneGraph
    Magnum::Shaders
    MagnumIntegration::ImGui
    )
    #    Magnum::Trade)


#add_dependencies(EyelashVisualizer
#    Magnum::AnyImageImporter
#    Magnum::AnySceneImporter
#    Magnum::ObjImporter
#    MagnumPlugins::StbImageImporter
#    MagnumPlugins::TinyGltfImporter)


# Make the executable a default target to build & run in Visual Studio
set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MyApplication)
