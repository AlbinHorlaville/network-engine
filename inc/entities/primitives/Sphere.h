//
// Created by User on 14/03/2025.
//

#ifndef SPHERE_H
#define SPHERE_H

#include "entities/GameObject.h"
#include <string>

class Sphere: public GameObject {
public:
    float _scale;
    Sphere() = delete;
    Sphere(Level_1* app, std::string name, Object3D *parent, float scale, float mass, const Color3& color);
    Sphere(Level_1* app, Object3D *parent, float scale, float mass, const Color3& color);
    ~Sphere() = default;

    void setScale(float newScale);
    void setMass(float mass) override;
    void updateDataFromBullet() override;
    void serialize(std::ostream &ostr) const override;
    void unserialize(std::istream &istr) override;

private:
    btSphereShape _collisionShape;
};

#endif //SPHERE_H
