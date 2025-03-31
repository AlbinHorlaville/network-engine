//
// Created by User on 14/03/2025.
//

#ifndef CUBE_H
#define CUBE_H

#include "entities/GameObject.h"
#include <string>

class Cube: public GameObject {
public:
    btVector3 _scale;
    Cube() = delete;
    Cube(Level_1* app, std::string name, Object3D *parent, btVector3 scale, float mass, const Color3& color);
    Cube(Level_1* app, Object3D *parent, btVector3 scale, float mass, const Color3& color);
    ~Cube() = default;

    void setScale(btVector3 newScale);
    void setMass(float mass) override;
    void updateDataFromBullet() override;
    void serialize(std::ostream &ostr) const override;
    void unserialize(std::istream &istr) override;

private:
    btBoxShape _collisionShape;
};



#endif //CUBE_H
