//
// Created by User on 14/03/2025.
//

#include "entities/primitives/Sphere.h"

Sphere::Sphere(std::string name, Object3D* parent, float scale, btDynamicsWorld& bWorld):
GameObject(std::move(name)), _collisionShape(btSphereShape{scale}) {
    _scale = scale;
    this->_rigidBody = new RigidBody{parent, 5, &_collisionShape, bWorld};
}

Sphere::Sphere(Object3D* parent, const float scale, btDynamicsWorld& bWorld):
GameObject("Sphere"), _collisionShape(btSphereShape{scale}) {
    _scale = scale;
    this->_rigidBody = new RigidBody{parent, 5, &_collisionShape, bWorld};
}

void Sphere::setScale(const float newScale) {
    _scale = newScale;

    // Remove old shape and add a new one using unique_ptr
    _collisionShape = btSphereShape{_scale};
    this->_rigidBody->rigidBody().setCollisionShape(&_collisionShape);  // Get the raw pointer for the RigidBody

    // Sync pose to ensure the new shape is aligned correctly with the existing body
    this->_rigidBody->syncPose();
}

