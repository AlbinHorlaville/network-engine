//
// Created by User on 14/03/2025.
//

#include <fstream>
#include <iostream>
#include <bits/fs_fwd.h>
#include <bits/fs_path.h>
#include <entities/primitives/Sphere.h>

#include "systems/physics/PhysicsWorld.h"
#include <systems/physics/ProjectileManager.h>
#include "systems/levels/Engine.h"
#include "components/RigidBody.h"
#include "entities/primitives/Cube.h"
#include <Magnum/ImGuiIntegration/Context.hpp>


Engine::Engine(const Arguments &arguments) : Platform::Application(arguments, NoCreate) {}

Engine::~Engine() {
    delete _drawables;
    delete _pWorld;
    delete _pProjectileManager;
    delete _camera;
    delete _cameraRig;
    delete _cameraObject;
}

void Engine::initSimulation() {
        /* Try 8x MSAA, fall back to zero samples if not possible. Enable only 2x
       MSAA if we have enough DPI. */
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

    // Render ImGui
    _sceneTreeUI->DrawSceneTree();

    // Show FPS
    ImGui::Begin("Performances", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("FPS : %f", fps_handler.get());
    ImGui::End();

    // 🔹 Fenêtre d'inspection de l'objet sélectionné
    if (_sceneTreeUI->_selectedObject) {
        ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

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

void Engine::tickMovments() {
    constexpr float moveSpeed = 10.f; // Adjust speed as needed
    const float deltaTime = _timeline.previousFrameDuration();

    if(_pressedKeys.count(Key::R)) {
        _pProjectileManager->_shootSphere = false;
    }

    /* Movement */
    if (_pressedKeys.count(Key::W))
        _cameraRig->translateLocal(Vector3::zAxis(-moveSpeed * deltaTime));
    if (_pressedKeys.count(Key::S))
        _cameraRig->translateLocal(Vector3::zAxis(moveSpeed * deltaTime));
    if (_pressedKeys.count(Key::A))
        _cameraRig->translateLocal(Vector3::xAxis(-moveSpeed * deltaTime));
    if (_pressedKeys.count(Key::D))
        _cameraRig->translateLocal(Vector3::xAxis(moveSpeed * deltaTime));
    if (_pressedKeys.count(Key::Q))
        _cameraRig->translateLocal(Vector3::yAxis(moveSpeed * deltaTime));
    if (_pressedKeys.count(Key::E))
        _cameraRig->translateLocal(Vector3::yAxis(-moveSpeed * deltaTime));
}

void Engine::cleanWorld() {
    _pWorld->cleanWorld();

    // Remove object if their _rigidBody have been destroyed
    for (auto it = _objects.begin(); it != _objects.end(); ) {
        GameObject* object = it->second;
        object->updateDataFromBullet();
        if (object && object->_rigidBody && object->_rigidBody->_bRigidBody) {
            if (object->_rigidBody->_bRigidBody->getWorldTransform().getOrigin().length() > 99.0f) {
                if (_sceneTreeUI->_selectedObject == object) {
                    _sceneTreeUI->_selectedObject = nullptr;
                }
                _linkingContext.Unregister(object);
                it = _objects.erase(it);
                continue;
            }
        }
        ++it;
    }
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
    tickMovments();
    cleanWorld();

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
    fps_handler.update();
}

void Engine::keyPressEvent(KeyEvent& event) {
    _pressedKeys.insert(event.key());

    // Sérialiser tous les objets
    if (event.key() == Key::U) {
        std::ofstream fileOut("objects.bin", std::ios::binary);
        serialize(fileOut);
        for (auto it = _objects.begin(); it != _objects.end(); ) {
            auto obj = it->second;
            _linkingContext.Unregister(obj);
            delete obj;
            it = _objects.erase(it);
        }
        fileOut.close();
    }

    // Désérialiser tous les objets
    if (event.key() == Key::I) {
        std::ifstream fileIn("objects.bin", std::ios::binary);
        if (fileIn) {  // Vérifie si le fichier a été ouvert avec succès
            unserialize(fileIn);
        } else {
            std::cerr << "Erreur : le fichier objects.bin n'existe pas." << std::endl;
        }
        fileIn.close();
    }

    event.setAccepted();
}

void Engine::keyReleaseEvent(KeyEvent& event) {
    _pressedKeys.erase(event.key()); // Remove key when released
    event.setAccepted();
}

void Engine::pointerPressEvent(PointerEvent& event) {
    if(_imgui.handlePointerPressEvent(event)) return;

    /* Shoot an object on click */
    if(!event.isPrimary() ||
       !(event.pointer() & Pointer::MouseLeft))
        return;

    /* First scale the position from being relative to window size to being
       relative to framebuffer size as those two can be different on HiDPI
       systems */
    const Vector2 position = event.position()*Vector2{framebufferSize()}/Vector2{windowSize()};
    const Vector2 clickPoint = Vector2::yScale(-1.0f)*(position/Vector2{framebufferSize()} - Vector2{0.5f})*_camera->projectionSize();
    btVector3 direction = btVector3((_cameraObject->absoluteTransformation().rotationScaling() * Vector3{clickPoint, -1.0f}).
        normalized());
    Vector3 translate = _cameraObject->absoluteTransformation().translation();

    GameObject* projectile = _pProjectileManager->Shoot(this, &_scene, translate, direction);
    projectile->setMass(1000);
    //projectile->_location = btVector3(translate);
    addObject(projectile);

    event.setAccepted();
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