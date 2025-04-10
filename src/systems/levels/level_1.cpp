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
#include "systems/levels/Level_1.h"
#include "components/RigidBody.h"
#include "entities/primitives/Cube.h"
#include <Magnum/ImGuiIntegration/Context.hpp>


Level_1::Level_1(const Arguments &arguments) : Platform::Application(arguments, NoCreate) {
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

    _imgui = Magnum::ImGuiIntegration::Context(Vector2{windowSize()} / dpiScaling(),
                                           windowSize(), framebufferSize());

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

    /* Create the ground */
    auto* ground = new Cube(this, "Floor", &_scene, {4.0f, 0.5f, 4.0f}, 0.f, 0xffffff_rgbf);
    _objects[ground->_name] = ground;

    /* Create boxes with random colors */
    Deg hue = 42.0_degf;
    for(Int i = 0; i != 2; ++i) {
        for(Int j = 0; j != 2; ++j) {
            for(Int k = 0; k != 2; ++k) {
                Color3 color = Color3::fromHsv({hue += 137.5_degf, 0.75f, 0.9f});
                auto* o = new Cube(this, &_scene, {0.5f, 0.5f, 0.5f}, 3.f, color);
                _objects[o->_name] = o;
                o->_rigidBody->translate({i - 2.0f, j + 4.0f, k - 2.0f});
                o->_rigidBody->syncPose();
                // Register o in the Linking Context
                _linkingContext.Register(o);
            }
        }
    }

    _sceneTreeUI = new SceneTree(&_objects);

    /* Loop at 60 Hz max */
    setSwapInterval(1);
    setMinimalLoopPeriod(16.0_msec);
    _timeline.start();
}

Level_1::~Level_1() {
    delete _drawables;
    delete _pWorld;
    delete _pProjectileManager;
    delete _camera;
    delete _cameraRig;
    delete _cameraObject;
}


void Level_1::drawEvent() {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color|GL::FramebufferClear::Depth);

    // Start new ImGui frame
    _imgui.newFrame();

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
                it = _objects.erase(it);
                continue;
            }
        }
        ++it;
    }

    _pWorld->_bWorld->stepSimulation(_timeline.previousFrameDuration(), 5);

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

    // Render ImGui
    _sceneTreeUI->DrawSceneTree();

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

    swapBuffers();
    _timeline.nextFrame();
    redraw();
}

void Level_1::keyPressEvent(KeyEvent& event) {
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

void Level_1::keyReleaseEvent(KeyEvent& event) {
    _pressedKeys.erase(event.key()); // Remove key when released
    event.setAccepted();
}

void Level_1::pointerPressEvent(PointerEvent& event) {
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
    _objects[projectile->_name] = projectile;

    event.setAccepted();
}

void Level_1::pointerReleaseEvent(PointerEvent& event) {
    if(_imgui.handlePointerReleaseEvent(event)){}
}

void Level_1::pointerMoveEvent(PointerMoveEvent& event) {
    if(_imgui.handlePointerMoveEvent(event)){}
}

void Level_1::scrollEvent(ScrollEvent& event) {
    if(_imgui.handleScrollEvent(event)){}
}

void Level_1::serialize(std::ostream &ostr) const {
    // On sérialise le nombre d'objet que l'on sérialise
    uint16_t size_objects = _objects.size();
    ostr.write(reinterpret_cast<const char*>(&size_objects), sizeof(uint16_t));
    for (auto pair : _objects) {
        pair.second->serialize(ostr);
    }
}

void Level_1::unserialize(std::istream &istr) {
    uint16_t size_objects;
    istr.read(reinterpret_cast<char*>(&size_objects), sizeof(uint16_t));
    for (uint16_t i = 0; i < size_objects; i++) {
        // Désérialiser l'ID d'objet.
        uint32_t id;
        istr.read(reinterpret_cast<char*>(&id), sizeof(uint32_t));

        GameObject* obj = _linkingContext.GetLocalObject(id);
        if (obj) { // L'objet est trouvé
            ObjectType type;
            istr.read(reinterpret_cast<char*>(&type), sizeof(ObjectType));
            obj->unserialize(istr);
        }
        else { // L'objet doit être créé
            ObjectType type;
            istr.read(reinterpret_cast<char*>(&type), sizeof(ObjectType));
            switch(type) {
                case CUBE: {
                    auto cube = new Cube(this, &_scene);
                    cube->unserialize(istr);
                    _objects[cube->_name] = cube;
                    _linkingContext.Register(cube);
                    break;
                }
                case SPHERE: {
                    auto* sphere = new Sphere(this, &_scene);
                    sphere->unserialize(istr);
                    _objects[sphere->_name] = sphere;
                    _linkingContext.Register(sphere);
                    break;
                }
            }
        }
    }
}

