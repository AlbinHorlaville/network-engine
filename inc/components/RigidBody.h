//
// Created by Albin Horlaville and Hugo Girard on 14/03/2025.
//

#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Corrade/Containers/Pointer.h>
#include <Magnum/BulletIntegration/Integration.h>
#include <Magnum/BulletIntegration/MotionState.h>
#include <btBulletDynamicsCommon.h>
#include "systems/physics/PhysicsWorld.h"

using namespace Magnum;

using Object3D = SceneGraph::Object<SceneGraph::MatrixTransformation3D>;

class RigidBody : public Object3D {
public:
    RigidBody(Object3D* parent, Float mass, btCollisionShape* bShape, PhysicsWorld& world);
    ~RigidBody();
    btRigidBody& rigidBody();

    void syncPose();

private:
    PhysicsWorld& _physicsWorld;
    Containers::Pointer<btRigidBody> _bRigidBody;
};



#endif //RIGIDBODY_H
