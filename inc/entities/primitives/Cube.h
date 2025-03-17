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
    Cube(Level_1* app, std::string name, Object3D *parent, btVector3 scale, float mass, const Color3& color);
    Cube(Level_1* app, Object3D *parent, btVector3 scale, float mass, const Color3& color);
    ~Cube() = default;

    void setScale(btVector3 newScale);
    void setMass(float mass) override;

private:
    btBoxShape _collisionShape;
    btVector3 _scale;
    float _mass;
};



#endif //CUBE_H
