//
// Created by User on 14/03/2025.
//

#ifndef PROJECTILEMANAGER_H
#define PROJECTILEMANAGER_H

#include <Magnum/SceneGraph/Scene.h>

class btVector3;
class GameObject;
class PhysicsObject;
class Engine;

using namespace Magnum;
typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;

class ProjectileManager {
public:
    bool _shootSphere;

    ProjectileManager(){_shootSphere = true;}
    GameObject* Shoot(Engine* app, Scene3D* scene, Vector3& position, btVector3& direction);
};



#endif //PROJECTILEMANAGER_H
