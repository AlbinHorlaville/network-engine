//
// Created by User on 14/03/2025.
//

#ifndef ENGINE_H
#define ENGINE_H

#include <array>
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
#include <map>
#include <unordered_set>
#include <systems/editor/SceneTree.h>
#include <systems/network/Serializable.h>
#include "systems/network/LinkingContext.h"
#include "Magnum/ImGuiIntegration/Context.h"
#include "systems/network/FPSHandler.h"

class GameObject;
class PhysicsWorld;
class ProjectileManager;

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

struct DeltaTimer {
    std::chrono::high_resolution_clock::time_point lastFrameTime;

    float get() {
        auto now = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(now - lastFrameTime).count();
        lastFrameTime = now;
        return deltaTime;
    }
};

class Player;

class Engine: public Platform::Application, public Serializable {
    public:
      explicit Engine(const Arguments &arguments);
      ~Engine();

    void initSimulation();

    virtual void networkUpdate() {} // A override par Server et Client
    void serialize(std::ostream &ostr) const override = 0;
    void unserialize(std::istream &istr) override = 0;
    void addObject(GameObject* object);
    void addObject(GameObject* object, uint32_t id);
    void drawImGUI();
    void drawGraphics();
    void drawEvent() override;
    void tickEvent() override;
    void keyPressEvent(KeyEvent& event) override;
    void keyReleaseEvent(KeyEvent& event) override;
    void pointerPressEvent(PointerEvent& event) override;
    void scrollEvent(ScrollEvent& event) override;
    void pointerReleaseEvent(PointerEvent& event) override;
    void pointerMoveEvent(PointerMoveEvent& event) override;
    virtual void reset();

    GL::Mesh _box{NoCreate}, _sphere{NoCreate};
    GL::Buffer _boxInstanceBuffer{NoCreate}, _sphereInstanceBuffer{NoCreate};
    Shaders::PhongGL _shader{NoCreate};
    Containers::Array<InstanceData> _boxInstanceData, _sphereInstanceData;

    ImGuiIntegration::Context _imgui{NoCreate};  // ImGui context

    PhysicsWorld* _pWorld;
    ProjectileManager* _pProjectileManager;

    Scene3D _scene{};
    std::array<Player*, 4> _players = { nullptr };
    FPSHandler fps_handler;
    DeltaTimer deltaTime;
    SceneGraph::Camera3D* _camera;
    SceneGraph::DrawableGroup3D* _drawables;
    Timeline _timeline;

    Object3D *_cameraRig, *_cameraObject;

    btBoxShape _bBoxShape{{0.5f, 0.5f, 0.5f}};
    btSphereShape _bSphereShape{0.25f};
    btBoxShape _bGroundShape{{4.0f, 0.5f, 4.0f}};

    bool _drawCubes{true};

    std::unordered_map<std::string, GameObject*> _objects;

    SceneTree* _sceneTreeUI;
    LinkingContext _linkingContext;

    std::unordered_map<std::string, GameObject *> const& getObjects() const {
        return _objects;
    }
    Containers::Array<InstanceData>& getBoxInstanceData() {
        return _boxInstanceData;
    }

    Containers::Array<InstanceData>& getSphereInstanceData() {
        return _sphereInstanceData;
    }

    PhysicsWorld& getWorld() const {
        return *_pWorld;
    }

    SceneGraph::DrawableGroup3D* getDrawables() {
        return _drawables;
    }
};



#endif //ENGINE_H
