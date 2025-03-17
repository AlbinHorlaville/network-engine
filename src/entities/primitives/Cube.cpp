//
// Created by User on 14/03/2025.
//

#include <utility>

#include "entities/primitives/Cube.h"
#include "Magnum/Primitives/Cube.h"

Cube::Cube(Level_1* app, std::string name, Object3D *parent, const btVector3 scale, float mass, const Color3& color):
    GameObject(std::move(name)), _collisionShape(btBoxShape{scale}) {
    _app = app;
    _mass = mass;
    _scale = scale;

    // Physics
    this->_rigidBody = new RigidBody{parent, _mass, &_collisionShape, _app->getWorld()};

    // Appearances
    new ColoredDrawable{*(_rigidBody), _app->getBoxInstanceData(), color,
    Matrix4::scaling(Vector3{_scale}), _app->getDrawables()};
}

Cube::Cube(Level_1* app, Object3D* parent, const btVector3 scale, const float mass, const Color3& color):
    GameObject("Cube"), _collisionShape(btBoxShape{scale}) {
    _app = app;
    _mass = mass;
    _scale = scale;

    // Physics
    this->_rigidBody = new RigidBody{parent, _mass, &_collisionShape, _app->getWorld()};

    // Appearance
    new ColoredDrawable{*(_rigidBody), _app->getBoxInstanceData(), color,
    Matrix4::scaling(Vector3{_scale}), _app->getDrawables()};
}

void Cube::setScale(btVector3 newScale) {
    _scale = newScale;

    // Remove old shape and add a new one using unique_ptr
    _collisionShape = btBoxShape{_scale};  // Use unique_ptr to manage the new shape
    this->_rigidBody->rigidBody().setCollisionShape(&_collisionShape);  // Get the raw pointer for the RigidBody
    // Sync pose to ensure the new shape is aligned correctly with the existing body
    this->_rigidBody->syncPose();
}

void Cube::setMass(const float mass) {
    _mass = mass;

    // Compute the inertia tensor for a cube with non-uniform scaling
    const btVector3 scaleVec = _scale * 2.0f; // Assuming _scale is the half side length for each dimension

    const float I_x = (1.0f / 12.0f) * mass * (scaleVec.y() * scaleVec.y() + scaleVec.z() * scaleVec.z());
    const float I_y = (1.0f / 12.0f) * mass * (scaleVec.x() * scaleVec.x() + scaleVec.z() * scaleVec.z());
    const float I_z = (1.0f / 12.0f) * mass * (scaleVec.x() * scaleVec.x() + scaleVec.y() * scaleVec.y());

    // Create the inertia tensor (diagonal components for a non-uniformly scaled cube)
    const btVector3 inertia(I_x, I_y, I_z);

    // Set mass properties with mass and inertia tensor
    this->_rigidBody->rigidBody().setMassProps(_mass, inertia);
}
