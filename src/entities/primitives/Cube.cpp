//
// Created by User on 14/03/2025.
//

#include <utility>

#include "entities/primitives/Cube.h"

Cube::Cube(std::string name, Object3D *parent, btDynamicsWorld &bWorld): GameObject(std::move(name)) {
    btBoxShape bShape{{0.5f, 0.5f, 0.5f}};
    this->_rigidBody = new RigidBody{parent, 1, &bShape, bWorld};
}

Cube::Cube(Object3D* parent, btDynamicsWorld& bWorld): GameObject("Cube") {
    btBoxShape bShape{{0.5f, 0.5f, 0.5f}};
    this->_rigidBody = new RigidBody{parent, 1, &bShape, bWorld};
}
