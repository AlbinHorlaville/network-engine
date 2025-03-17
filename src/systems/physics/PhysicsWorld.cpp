//
// Created by User on 14/03/2025.
//

#include "systems/physics/PhysicsWorld.h"

#include <vector>

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

void PhysicsWorld::cleanWorld() const {
    btCollisionObjectArray& collisionObjects = _bWorld->getCollisionObjectArray(); // Use reference to avoid copy
    std::vector<btRigidBody*> toRemove; // Store objects to remove after iteration

    for (int i = 0; i < collisionObjects.size(); ++i) {
        btCollisionObject* obj = collisionObjects[i];
        btRigidBody* body = btRigidBody::upcast(obj);

        if (body && body->getWorldTransform().getOrigin().length() > 100.0) { // Correct distance check
            toRemove.push_back(body);
        }
    }

    // Now safely remove and delete objects
    for (btRigidBody* body : toRemove) {
        _bWorld->removeRigidBody(body);
        delete body;
    }
}

