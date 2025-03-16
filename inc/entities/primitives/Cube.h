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
    Cube(std::string name, Object3D *parent, btDynamicsWorld &bWorld);
    Cube(Object3D *parent, btDynamicsWorld &bWorld);
    ~Cube(){}
};



#endif //CUBE_H
