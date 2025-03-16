//
// Created by User on 14/03/2025.
//

#ifndef SPHERE_H
#define SPHERE_H

#include "entities/GameObject.h"
#include <string>

class Sphere: public GameObject {
public:
    Sphere() = delete;
    Sphere(std::string name, Object3D *parent, float scale, float mass, btDynamicsWorld &bWorld);
    Sphere(Object3D *parent, float scale, float mass, btDynamicsWorld &bWorld);
    ~Sphere() = default;

    void setScale(float newScale);
    void setMass(float mass);

private:
    btSphereShape _collisionShape;
    float _scale;
    float _mass;
};

#endif //SPHERE_H
