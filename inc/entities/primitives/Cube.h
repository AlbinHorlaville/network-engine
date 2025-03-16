//
// Created by User on 14/03/2025.
//

#ifndef CUBE_H
#define CUBE_H

#include "entities/GameObject.h"
#include <string>

class Cube: public GameObject {
public:
    Cube() = delete;
    Cube(std::string name, Object3D *parent, btVector3 scale, float mass, btDynamicsWorld &bWorld);
    Cube(Object3D *parent, btVector3 scale, float mass, btDynamicsWorld &bWorld);
    ~Cube() = default;

    void setScale(btVector3 newScale);
    void setMass(float mass);

private:
    btBoxShape _collisionShape;
    btVector3 _scale;
    float _mass;
};



#endif //CUBE_H
