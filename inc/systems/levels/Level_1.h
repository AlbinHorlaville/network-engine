//
// Created by User on 14/03/2025.
//

#ifndef LEVEL_1_H
#define LEVEL_1_H

#include <btBulletDynamicsCommon.h>
#include <Corrade/Containers/GrowableArray.h>
#include <Corrade/Containers/Optional.h>
#include <Corrade/Containers/Pointer.h>
#include <Magnum/Timeline.h>
#include <Magnum/BulletIntegration/Integration.h>
#include <Magnum/BulletIntegration/MotionState.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/Math/Constants.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Time.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/MeshTools/Transform.h>
#include <Magnum/Platform/GlfwApplication.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/Primitives/UVSphere.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/Shaders/PhongGL.h>
#include <Magnum/Trade/MeshData.h>
#include <list>
#include <unordered_set>
#include <systems/physics/ProjectileManager.h>

#include "entities/GameObject.h"
#include "systems/physics/PhysicsWorld.h"

using namespace Magnum;
using namespace Math::Literals;
using KeyEvent = Platform::Application::KeyEvent;

typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;
typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;

struct InstanceData {
    Matrix4 transformationMatrix;
    Matrix3x3 normalMatrix;
    Color3 color;
};

class Level_1: public Platform::Application {
    public:
      explicit Level_1(const Arguments &arguments);

    private:
      void drawEvent() override;
      void keyPressEvent(KeyEvent& event) override;
      void keyReleaseEvent(KeyEvent& event) override;
      void pointerPressEvent(PointerEvent& event) override;

    GL::Mesh _box{NoCreate}, _sphere{NoCreate};
    GL::Buffer _boxInstanceBuffer{NoCreate}, _sphereInstanceBuffer{NoCreate};
    Shaders::PhongGL _shader{NoCreate};
    Containers::Array<InstanceData> _boxInstanceData, _sphereInstanceData;

    PhysicsWorld* _pWorld;
    ProjectileManager* _pProjectileManager;

    Scene3D _scene;
    SceneGraph::Camera3D* _camera;
    SceneGraph::DrawableGroup3D _drawables;
    Timeline _timeline;

    Object3D *_cameraRig, *_cameraObject;

    btBoxShape _bBoxShape{{0.5f, 0.5f, 0.5f}};
    btSphereShape _bSphereShape{0.25f};
    btBoxShape _bGroundShape{{4.0f, 0.5f, 4.0f}};

    bool _drawCubes{true};

    std::list<GameObject*> _objects;
    std::unordered_set<KeyEvent::Key> _pressedKeys;
};

class ColoredDrawable: public SceneGraph::Drawable3D {
public:
    explicit ColoredDrawable(Object3D& object, Containers::Array<InstanceData>& instanceData, const Color3& color, const Matrix4& primitiveTransformation, SceneGraph::DrawableGroup3D& drawables): SceneGraph::Drawable3D{object, &drawables}, _instanceData(instanceData), _color{color}, _primitiveTransformation{primitiveTransformation} {}

private:
    void draw(const Matrix4& transformation, SceneGraph::Camera3D&) override {
        const Matrix4 t = transformation*_primitiveTransformation;
        arrayAppend(_instanceData, InPlaceInit, t, t.normalMatrix(), _color);
    }

    Containers::Array<InstanceData>& _instanceData;
    Color3 _color;
    Matrix4 _primitiveTransformation;
};



#endif //LEVEL_1_H
