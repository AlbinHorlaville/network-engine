//
// Created by User on 14/03/2025.
//

#include <iostream>
#include <entities/primitives/Sphere.h>

#include "systems\levels\Level_1.h"
#include "components\Rigidbody.h"
#include "entities/primitives/Cube.h"

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

    /* Camera setup */
    (*(_cameraRig = new Object3D{&_scene}))
        .translate(Vector3::yAxis(3.0f))
        .rotateY(40.0_degf);
    (*(_cameraObject = new Object3D{_cameraRig}))
        .translate(Vector3::zAxis(20.0f))
        .rotateX(-25.0_degf);
    (_camera = new SceneGraph::Camera3D(*_cameraObject))
        ->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
        .setProjectionMatrix(Matrix4::perspectiveProjection(35.0_degf, 1.0f, 0.001f, 100.0f))
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

    /* Create the ground */
    auto* ground = new RigidBody{&_scene, 0.0f, &_bGroundShape, *(_pWorld->_bWorld)};
    new ColoredDrawable{*ground, _boxInstanceData, 0xffffff_rgbf,
        Matrix4::scaling({4.0f, 0.5f, 4.0f}), _drawables};

    /* Create boxes with random colors */
    Deg hue = 42.0_degf;
    for(Int i = 0; i != 5; ++i) {
        for(Int j = 0; j != 5; ++j) {
            for(Int k = 0; k != 5; ++k) {
                auto* o = new Cube(&_scene, {0.5f, 0.5f, 0.5f},*(_pWorld->_bWorld));
                _objects.push_back(o);
                //auto* o = new RigidBody{&_scene, 1.0f, &_bBoxShape, *(_pWorld->_bWorld)};
                o->_rigidBody->translate({i - 2.0f, j + 4.0f, k - 2.0f});
                o->_rigidBody->syncPose();
                new ColoredDrawable{*(o->_rigidBody), _boxInstanceData,
                    Color3::fromHsv({hue += 137.5_degf, 0.75f, 0.9f}),
                    Matrix4::scaling(Vector3{0.5f}), _drawables};
            }
        }
    }

    /* Loop at 60 Hz max */
    setSwapInterval(1);
    setMinimalLoopPeriod(16.0_msec);
    _timeline.start();
}

void Level_1::drawEvent() {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color|GL::FramebufferClear::Depth);

    constexpr float moveSpeed = 10.f; // Adjust speed as needed
    const float deltaTime = _timeline.previousFrameDuration();

    /* Movement */
    if (_pressedKeys.count(KeyEvent::Key::W))
        _cameraRig->translateLocal(Vector3::zAxis(-moveSpeed * deltaTime));
    if (_pressedKeys.count(KeyEvent::Key::S))
        _cameraRig->translateLocal(Vector3::zAxis(moveSpeed * deltaTime));
    if (_pressedKeys.count(KeyEvent::Key::A))
        _cameraRig->translateLocal(Vector3::xAxis(-moveSpeed * deltaTime));
    if (_pressedKeys.count(KeyEvent::Key::D))
        _cameraRig->translateLocal(Vector3::xAxis(moveSpeed * deltaTime));
    if (_pressedKeys.count(KeyEvent::Key::Q))
        _cameraRig->translateLocal(Vector3::yAxis(moveSpeed * deltaTime));
    if (_pressedKeys.count(KeyEvent::Key::E))
        _cameraRig->translateLocal(Vector3::yAxis(-moveSpeed * deltaTime));

    /* Housekeeping: remove any objects which are far away from the origin */
    for(Object3D* obj = _scene.children().first(); obj; )
    {
        Object3D* next = obj->nextSibling();
        if(obj->transformation().translation().dot() > 100*100) {
            if (auto* rigidBody = dynamic_cast<RigidBody*>(obj)) {
                _pWorld->_bWorld->removeRigidBody(&rigidBody->rigidBody());  // Remove safely
            }
            delete obj;  // Now it's safe to delete
        }

        obj = next;
    }

    // Remove object if there _rigidBody have been destroyed
    for (GameObject *object : _objects) {
        if (!object->_rigidBody) {
            _objects.erase(std::find(_objects.begin(), _objects.end(), object));
            delete object;
        }
    }

    _pWorld->_bWorld->stepSimulation(_timeline.previousFrameDuration(), 5);

    if(_drawCubes) {
        /* Populate instance data with transformations and colors */
        arrayResize(_boxInstanceData, 0);
        arrayResize(_sphereInstanceData, 0);
        _camera->draw(_drawables);

        _shader.setProjectionMatrix(_camera->projectionMatrix());

        /* Upload instance data to the GPU (orphaning the previous buffer
           contents) and draw all cubes in one call, and all spheres (if any)
           in another call */
        _boxInstanceBuffer.setData(_boxInstanceData, GL::BufferUsage::DynamicDraw);
        _box.setInstanceCount(_boxInstanceData.size());
        _shader.draw(_box);

        _sphereInstanceBuffer.setData(_sphereInstanceData, GL::BufferUsage::DynamicDraw);
        _sphere.setInstanceCount(_sphereInstanceData.size());
        _shader.draw(_sphere);
    }

    swapBuffers();
    _timeline.nextFrame();
    redraw();
}

void Level_1::keyPressEvent(KeyEvent& event) {
    _pressedKeys.insert(event.key());
    event.setAccepted();
}

void Level_1::keyReleaseEvent(KeyEvent& event) {
    _pressedKeys.erase(event.key()); // Remove key when released
    event.setAccepted();
}

void Level_1::pointerPressEvent(PointerEvent& event) {
    /* Shoot an object on click */
    if(!event.isPrimary() ||
       !(event.pointer() & Pointer::MouseLeft))
        return;

    /* First scale the position from being relative to window size to being
       relative to framebuffer size as those two can be different on HiDPI
       systems */
    const Vector2 position = event.position()*Vector2{framebufferSize()}/Vector2{windowSize()};
    const Vector2 clickPoint = Vector2::yScale(-1.0f)*(position/Vector2{framebufferSize()} - Vector2{0.5f})*_camera->projectionSize();
    const Vector3 direction = (_cameraObject->absoluteTransformation().rotationScaling()*Vector3{clickPoint, -1.0f}).normalized();

    auto* projectile = new Sphere(&_scene, 0.25f, *(_pWorld->_bWorld));
    _objects.push_back(projectile);
    /*
    auto* object = new RigidBody{
        &_scene,
        5.0f,
        &_bSphereShape,
        *(_pWorld->_bWorld)};
        **/
    projectile->_rigidBody->translate(_cameraObject->absoluteTransformation().translation());
    /* Has to be done explicitly after the translate() above, as Magnum ->
       Bullet updates are implicitly done only for kinematic bodies */
    projectile->_rigidBody->syncPose();

    /* Create either a box or a sphere */
    new ColoredDrawable{*(projectile->_rigidBody),
        _sphereInstanceData,
        0x220000_rgbf,
        Matrix4::scaling(Vector3{0.25f}), _drawables};

    /* Give it an initial velocity */
    projectile->_rigidBody->rigidBody().setLinearVelocity(btVector3{direction*25.f});

    event.setAccepted();
}
