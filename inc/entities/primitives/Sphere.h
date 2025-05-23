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
    Sphere(Engine* app, std::string name, Object3D *parent, float scale, float mass, const Color3& color);
    Sphere(Engine* app, Object3D *parent, float scale, float mass, const Color3& color);
    Sphere(Engine* app, Object3D* parent);
    ~Sphere() = default;

    void setScale(float newScale);
    void setMass(float mass) override;
    void setColor(const Color3 &color) override;
    void updateBulletFromData() override;
    void serialize(std::ostream &ostr) const override;
    void unserialize(std::istream &istr) override;

private:
    btSphereShape _collisionShape;
};

#endif //SPHERE_H
