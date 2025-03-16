//
// Created by User on 14/03/2025.
//

#include "systems/physics/PhysicsWorld.h"

PhysicsWorld::PhysicsWorld(const btVector3 gravity) {
    _bBroadphase = std::make_unique<btDbvtBroadphase>();
    _bCollisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
    _bDispatcher = std::make_unique<btCollisionDispatcher>(_bCollisionConfig.get());
    _bSolver = std::make_unique<btSequentialImpulseConstraintSolver>();

    _bWorld = std::make_unique<btDiscreteDynamicsWorld>(
        _bDispatcher.get(), _bBroadphase.get(),
        _bSolver.get(), _bCollisionConfig.get());

    _bWorld->setGravity(gravity);
}

void PhysicsWorld::setGravity(const btVector3 gravity) const {
    _bWorld->setGravity(gravity);
}
