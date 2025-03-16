//
// Created by User on 14/03/2025.
//

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <string>
#include <utility>
#include <Magnum/GL/Mesh.h>
#include "components/RigidBody.h"

class GameObject {
public:
    std::string _name;
    RigidBody* _rigidBody;

    GameObject(std::string name) : _name(std::move(name)), _rigidBody(nullptr) {}

    ~GameObject() {delete _rigidBody;}
};



#endif //GAMEOBJECT_H
