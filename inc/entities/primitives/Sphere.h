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
    Sphere(std::string name, Object3D *parent, btDynamicsWorld &bWorld);
    Sphere(Object3D *parent, btDynamicsWorld &bWorld);
    Sphere(Object3D *parent, btCollisionShape *shape, btDynamicsWorld &bWorld);
    ~Sphere(){};
};



#endif //SPHERE_H
