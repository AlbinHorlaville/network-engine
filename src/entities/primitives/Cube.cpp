//
// Created by User on 14/03/2025.
//

#include <utility>

#include "entities/primitives/Cube.h"

Cube::Cube(std::string name, Object3D *parent, const btVector3 scale, btDynamicsWorld &bWorld):
GameObject(std::move(name)), _collisionShape(btBoxShape{scale}) {
    _scale = scale;
    this->_rigidBody = new RigidBody{parent, 1, &_collisionShape, bWorld};
}

Cube::Cube(Object3D* parent, const btVector3 scale, btDynamicsWorld& bWorld):
GameObject("Cube"), _collisionShape(btBoxShape{scale}) {
    _scale = scale;
    this->_rigidBody = new RigidBody{parent, 1, &_collisionShape, bWorld};
}

void Cube::setScale(btVector3 newScale) {
    _scale = newScale;

    // Remove old shape and add a new one using unique_ptr
    _collisionShape = btBoxShape{_scale};  // Use unique_ptr to manage the new shape
    this->_rigidBody->rigidBody().setCollisionShape(&_collisionShape);  // Get the raw pointer for the RigidBody

    // Sync pose to ensure the new shape is aligned correctly with the existing body
    this->_rigidBody->syncPose();
}
