//
// Created by User on 14/03/2025.
//

#include <fstream>
#include <iostream>
#include <entities/primitives/Sphere.h>

#include "systems/physics/PhysicsWorld.h"
#include <systems/physics/ProjectileManager.h>
#include "systems/levels/Engine.h"
#include "components/RigidBody.h"
#include "entities/primitives/Cube.h"
#include <Magnum/ImGuiIntegration/Context.hpp>
#include "entities/primitives/Player.h"


Engine::Engine(const Arguments &arguments) : Platform::Application(arguments, NoCreate) {
    {
        const Vector2 dpiScaling = this->dpiScaling({});
        Configuration conf;
        conf.setTitle("Magnum Bullet Integration Example")
            .setSize(conf.size(), dpiScaling);
        GLConfiguration glConf;
        glConf.setSampleCount(dpiScaling.max() < 2.0f ? 8 : 2);
        if(!tryCreate(conf, glConf))
            create(conf, glConf.setSampleCount(0));
    }

    _imgui = Magnum::ImGuiIntegration::Context(Vector2{windowSize()} / dpiScaling(),
                                           windowSize(), framebufferSize());

    GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add,
    GL::Renderer::BlendEquation::Add);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
    GL::Renderer::BlendFunction::OneMinusSourceAlpha);
    fps_handler.init();
    startTextInput();
}

Engine::~Engine() {
    delete _drawables;
    delete _pWorld;
    delete _pProjectileManager;
    delete _camera;
    delete _cameraRig;
    delete _cameraObject;
}

void Engine::initSimulation() {
    /* Camera setup */
    (*(_cameraRig = new Object3D{&_scene}))
        .translate(Vector3::yAxis(3.0f))
        .rotateY(40.0_degf);
    (*(_cameraObject = new Object3D{_cameraRig}))
        .translate(Vector3::zAxis(20.0f))
        .rotateX(-25.0_degf);
    (_camera = new SceneGraph::Camera3D(*_cameraObject))
        ->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
        .setProjectionMatrix(Matrix4::perspectiveProjection(35.0_degf, 1.0f, 0.001f, 99.0f))
        .setViewport(GL::defaultFramebuffer.viewport().size());

    /* Create an instanced shader */
    _shader = Shaders::PhongGL{Shaders::PhongGL::Configuration{}
        .setFlags(Shaders::PhongGL::Flag::VertexColor|
                  Shaders::PhongGL::Flag::InstancedTransformation)};
    _shader.setAmbientColor(0x111111_rgbf)
           .setSpecularColor(0x330000_rgbf)
           .setLightPositions({{10.0f, 15.0f, 5.0f, 0.0f}});

    /* Box and sphere mesh, with an (initially empty) instance buffer */
    _box = MeshTools::compile(Primitives::cubeSolid());
    _sphere = MeshTools::compile(Primitives::uvSphereSolid(16, 32));
    _boxInstanceBuffer = GL::Buffer{};
    _sphereInstanceBuffer = GL::Buffer{};
    _box.addVertexBufferInstanced(_boxInstanceBuffer, 1, 0,
        Shaders::PhongGL::TransformationMatrix{},
        Shaders::PhongGL::NormalMatrix{},
        Shaders::PhongGL::Color3{});
    _sphere.addVertexBufferInstanced(_sphereInstanceBuffer, 1, 0,
        Shaders::PhongGL::TransformationMatrix{},
        Shaders::PhongGL::NormalMatrix{},
        Shaders::PhongGL::Color3{});

    /* Setup the renderer so we can draw the debug lines on top */
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
    GL::Renderer::enable(GL::Renderer::Feature::PolygonOffsetFill);
    GL::Renderer::setPolygonOffset(2.0f, 0.5f);

    /* Bullet setup */
    _pWorld = new PhysicsWorld({0.0f, -10.0f, 0.0f});
    _pProjectileManager = new ProjectileManager();

    // Init drawables
    _drawables = new SceneGraph::DrawableGroup3D();

    _sceneTreeUI = new SceneTree(&_objects);

    /* Loop at 60 Hz max */
    setSwapInterval(1);
    setMinimalLoopPeriod(16.0_msec);
    _timeline.start();

    // Init FPS handler
    fps_handler.init();
}

void Engine::drawImGUI() {
    // Start new ImGui frame
    _imgui.newFrame();
    fps_handler.update();

    // Render ImGui
    _sceneTreeUI->DrawSceneTree();

    // Show FPS
    bool open = ImGui::Begin("Performances", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    if (open) {
        for (auto &player : _players) {
            if (player) {
                ImGui::Text("%s | FPS : %d, Ping : %d ms", player->_name.c_str(), player->_fps, player->_ping);
            }
        }
    }
    ImGui::End();

    // LeaderBoard
    open = ImGui::Begin("Leaderboard", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    if (open) {
        const int size = static_cast<int>(_players.size());
        for (int i = 0; i<size; i++) {
            if (_players[i]) {
                ImGui::Text("%s : %d", _players[i]->_name.c_str(), _players[i]->_score);
            }
        }
        /*
        ImVec2 windowSize = ImGui::GetIO().DisplaySize;
        ImVec2 panelSizeFPS = ImVec2(0.15f * windowSize.x, windowSize.y / 10.f);

        ImGui::SetNextWindowSize(panelSizeFPS);
        ImGui::SetNextWindowPos(
            ImVec2(windowSize.x - panelSizeFPS.x - 3.0f, 3.0f), // Top-right with a 3px margin
            ImGuiCond_Always,
            ImVec2(0.0f, 0.0f)
        );
        */
    }
    ImGui::End();

    // 🔹 Fenêtre d'inspection de l'objet sélectionné
    /*
    ImVec2 windowSize = ImGui::GetIO().DisplaySize;
    ImVec2 panelSizeInspector = ImVec2(0.32f * windowSize.x, windowSize.y / 5.f);

    ImGui::SetNextWindowSize(panelSizeInspector);
    ImGui::SetNextWindowPos(
        ImVec2(windowSize.x - panelSizeInspector.x - 3.0f, windowSize.y - panelSizeInspector.y - 3.0f), // Bottom-right with a 3px margin
        ImGuiCond_Always,
        ImVec2(0.0f, 0.0f)
    );
    */

    if (_sceneTreeUI->_selectedObject) {
        open = ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        if (open) {
            ImGui::Text("Name: %s", _sceneTreeUI->_selectedObject->_name.c_str());
            ImGui::Text("Position: (%.2f, %.2f, %.2f)",
                        static_cast<double>(_sceneTreeUI->_selectedObject->_location.x()),
                        static_cast<double>(_sceneTreeUI->_selectedObject->_location.y()),
                        static_cast<double>(_sceneTreeUI->_selectedObject->_location.z()));
            ImGui::Text("Rotation: (%.2f, %.2f, %.2f, %.2f)",
                static_cast<double>(_sceneTreeUI->_selectedObject->_rotation.x()),
                static_cast<double>(_sceneTreeUI->_selectedObject->_rotation.y()),
                static_cast<double>(_sceneTreeUI->_selectedObject->_rotation.z()),
                static_cast<double>(_sceneTreeUI->_selectedObject->_rotation.w()));
            if (auto cube = dynamic_cast<Cube*>(_sceneTreeUI->_selectedObject)) {
                ImGui::Text("Scale : (%.2f, %.2f, %.2f)",
                    static_cast<double>(cube->_scale.x()),
                    static_cast<double>(cube->_scale.y()),
                    static_cast<double>(cube->_scale.z()));
            }
            if (auto sphere = dynamic_cast<Sphere*>(_sceneTreeUI->_selectedObject)) {
                ImGui::Text("Scale : %.2f",
                    static_cast<double>(sphere->_scale));
            }
            ImGui::Text("Mass : %.2f",
                static_cast<double>(_sceneTreeUI->_selectedObject->_mass));
        }
        ImGui::End();
    }

    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);
    GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);
    GL::Renderer::disable(GL::Renderer::Feature::DepthTest);

    _imgui.drawFrame();

    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
    GL::Renderer::disable(GL::Renderer::Feature::ScissorTest);
    GL::Renderer::disable(GL::Renderer::Feature::Blending);
}

void Engine::drawGraphics() {
    if(_drawCubes) {
        /* Populate instance data with transformations and colors */
        arrayResize(_boxInstanceData, 0);
        arrayResize(_sphereInstanceData, 0);
        _camera->draw(*_drawables);

        _shader.setProjectionMatrix(_camera->projectionMatrix());

        _boxInstanceBuffer.setData(_boxInstanceData, GL::BufferUsage::DynamicDraw);
        _box.setInstanceCount(_boxInstanceData.size());
        _shader.draw(_box);

        _sphereInstanceBuffer.setData(_sphereInstanceData, GL::BufferUsage::DynamicDraw);
        _sphere.setInstanceCount(_sphereInstanceData.size());
        _shader.draw(_sphere);
    }
}

void Engine::tickEvent() {

    // Simulation physique
    _pWorld->_bWorld->stepSimulation(_timeline.previousFrameDuration(), 5);

    // Avance la timeline et redessine
    _timeline.nextFrame();
    redraw();
}

void Engine::drawEvent() {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color|GL::FramebufferClear::Depth);
    drawGraphics();
    drawImGUI();
    swapBuffers();
}

void Engine::keyPressEvent(KeyEvent& event) {
    event.setAccepted();
}

void Engine::keyReleaseEvent(KeyEvent& event) {
    event.setAccepted();
}

void Engine::pointerPressEvent(PointerEvent& event) {
    if(_imgui.handlePointerPressEvent(event)) return;
}

void Engine::pointerReleaseEvent(PointerEvent& event) {
    if(_imgui.handlePointerReleaseEvent(event)){}
}

void Engine::pointerMoveEvent(PointerMoveEvent& event) {
    if(_imgui.handlePointerMoveEvent(event)){}
}

void Engine::scrollEvent(ScrollEvent& event) {
    if(_imgui.handleScrollEvent(event)){}
}

void Engine::addObject(GameObject* object) {
    _objects[object->_name] = object;
    _linkingContext.Register(object);
}

void Engine::addObject(GameObject* object, uint32_t id) {
    _objects[object->_name] = object;
    _linkingContext.Register(id, object);
}