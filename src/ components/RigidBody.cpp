//
// Created by User on 14/03/2025.
//

#include "components/RigidBody.h"

class PhysicsWorld;

RigidBody::RigidBody(Object3D* parent, Float mass, btCollisionShape* bShape, PhysicsWorld& world): Object3D{parent}, _physicsWorld(world){
    /* Calculate inertia so the object reacts as it should with
     rotation and everything */
    btVector3 bInertia(0.0f, 0.0f, 0.0f);
    if(!Math::TypeTraits<Float>::equals(mass, 0.0f))
        bShape->calculateLocalInertia(mass, bInertia);

    /* Bullet rigid body setup */
    auto* motionState = new BulletIntegration::MotionState{*this};
    _bRigidBody.emplace(btRigidBody::btRigidBodyConstructionInfo{
        mass, &motionState->btMotionState(), bShape, bInertia});
    _bRigidBody->forceActivationState(DISABLE_DEACTIVATION);
    _physicsWorld._bWorld->addRigidBody(_bRigidBody.get());
}

RigidBody::~RigidBody() {
    _physicsWorld._bWorld->removeRigidBody(_bRigidBody.get());
}

btRigidBody& RigidBody::rigidBody() {
    return *_bRigidBody;
}

void RigidBody::syncPose() {
    _bRigidBody->setWorldTransform(btTransform(transformationMatrix()));
}