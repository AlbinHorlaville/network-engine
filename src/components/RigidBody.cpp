//
// Created by User on 14/03/2025.
//

#include "components/RigidBody.h"

class PhysicsWorld;

// Constructeur à appeler avant la déserialisation pour reconstruire l'objet
RigidBody::RigidBody(Object3D* parent, btCollisionShape* bShape, PhysicsWorld& world)
    : Object3D{parent}, _bShape(bShape), _physicsWorld(world)
{
    createBtRigidBody(1);
}

RigidBody::RigidBody(Object3D* parent, Float mass, btCollisionShape* bShape, PhysicsWorld& world)
    : Object3D{parent}, _bShape(bShape), _physicsWorld(world)
{
    createBtRigidBody(mass);
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

void RigidBody::createBtRigidBody(Float mass) {
    if (_physicsWorld._bWorld->getCollisionObjectArray().findLinearSearch(_bRigidBody.get()) != _physicsWorld._bWorld->getCollisionObjectArray().size()) {
        _physicsWorld._bWorld->removeRigidBody(_bRigidBody.get());
    }
    /* Calculate inertia so the object reacts as it should with
     rotation and everything */
    btVector3 bInertia(0.0f, 0.0f, 0.0f);
    if(!Math::TypeTraits<Float>::equals(mass, 0.0f))
        _bShape->calculateLocalInertia(mass, bInertia);

    /* Bullet rigid body setup */
    auto* motionState = new BulletIntegration::MotionState{*this};
    _bRigidBody.emplace(btRigidBody::btRigidBodyConstructionInfo{
        mass, &motionState->btMotionState(), _bShape, bInertia});
    _bRigidBody->forceActivationState(DISABLE_DEACTIVATION);
    _physicsWorld._bWorld->addRigidBody(_bRigidBody.get());
}
