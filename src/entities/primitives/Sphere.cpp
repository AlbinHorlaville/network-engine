//
// Created by User on 14/03/2025.
//

#include "entities/primitives/Sphere.h"

Sphere::Sphere(Level_1* app, std::string name, Object3D *parent, float scale, float mass, const Color3& color):
GameObject(std::move(name)), _collisionShape(btSphereShape{scale}) {
    _app = app;
    _mass = mass;
    _scale = scale;

    // Change name if it already exists
    giveDefaultName();

    // Physics
    this->_rigidBody = new RigidBody{parent, _mass, &_collisionShape, _app->getWorld()};

    // Appearance
    new ColoredDrawable{*(_rigidBody), _app->getSphereInstanceData(), color,
    Matrix4::scaling(Vector3{_scale}), _app->getDrawables()};
}

Sphere::Sphere(Level_1* app, Object3D* parent, const float scale, const float mass, const Color3& color):
    GameObject("Sphere"), _collisionShape(btSphereShape{scale}) {
    _app = app;
    _mass = mass;
    _scale = scale;

    // Change name if it already exists
    giveDefaultName();

    // Physics
    this->_rigidBody = new RigidBody{parent, _mass, &_collisionShape, _app->getWorld()};

    // Appearance
    new ColoredDrawable{*(_rigidBody), _app->getSphereInstanceData(), color,
    Matrix4::scaling(Vector3{_scale}), _app->getDrawables()};
}

void Sphere::setScale(const float newScale) {
    _scale = newScale;

    // Remove old shape and add a new one using unique_ptr
    _collisionShape = btSphereShape{_scale};
    this->_rigidBody->rigidBody().setCollisionShape(&_collisionShape);  // Get the raw pointer for the RigidBody

    // Sync pose to ensure the new shape is aligned correctly with the existing body
    this->_rigidBody->syncPose();
}

void Sphere::setMass(const float mass) {
    _mass = mass;

    // Compute the inertia tensor for a cube
    // Assuming the cube is aligned along the axes and centered at the origin.
    const float sideLength = _scale * 2.0f;  // _scale is the half side length
    const float I = mass * sideLength * sideLength / 6.0f;  // Inertia tensor for a cube

    // Create the inertia tensor (diagonal)
    const btVector3 inertia(I, I, I);  // For a cube, the inertia tensor is the same on all axes

    // Set mass properties with mass and inertia tensor
    this->_rigidBody->rigidBody().setMassProps(_mass, inertia);
}
