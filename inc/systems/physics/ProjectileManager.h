//
// Created by User on 14/03/2025.
//

#ifndef PROJECTILEMANAGER_H
#define PROJECTILEMANAGER_H

#include <entities/GameObject.h>
#include <Magnum/SceneGraph/Scene.h>
#include "systems/physics/PhysicsWorld.h"

using namespace Magnum;
typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;

class ProjectileManager {
public:
    bool _shootSphere;

    ProjectileManager(){_shootSphere = true;}
    GameObject* Shoot(Scene3D* scene, PhysicsWorld* physicsWorld, Vector3& position, btVector3& direction);
};



#endif //PROJECTILEMANAGER_H
