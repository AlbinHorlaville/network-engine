//
// Created by User on 14/03/2025.
//

#include "entities/primitives/Sphere.h"

Sphere::Sphere(std::string name, Object3D* parent, btDynamicsWorld& bWorld): GameObject(std::move(name)) {
    btSphereShape bShape{0.25f};
    this->_rigidBody = new RigidBody{parent, 1, &bShape, bWorld};
}

Sphere::Sphere(Object3D* parent, btDynamicsWorld& bWorld): GameObject("Sphere") {
    btSphereShape bShape{0.25f};
    this->_rigidBody = new RigidBody{parent, 1, &bShape, bWorld};
}

Sphere::Sphere(Object3D* parent, btCollisionShape *shape, btDynamicsWorld& bWorld): GameObject("Sphere") {
    this->_rigidBody = new RigidBody{parent, 1, shape, bWorld};
}
