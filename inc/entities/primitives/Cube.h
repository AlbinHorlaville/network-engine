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
    Cube(Engine* app, std::string name, Object3D *parent, btVector3 scale, float mass, const Color3& color);
    Cube(Engine* app, Object3D *parent, btVector3 scale, float mass, const Color3& color);
    Cube(Engine* app, Object3D *parent);
    ~Cube() = default;

    void setScale(btVector3 newScale);
    void setMass(float mass) override;
    void setColor(const Color3 &color) override;
    void updateBulletFromData() override;
    void serialize(std::ostream &ostr) const override;
    void unserialize(std::istream &istr) override;

private:
    btBoxShape _collisionShape;
};



#endif //CUBE_H
