//
// Created by User on 14/03/2025.
//

#include "entities/primitives/Sphere.h"
#include "Magnum/MeshTools/Compile.h"
#include "Magnum/Shaders/PhongGL.h"
#include "Magnum/Primitives/UVSphere.h"
#include "Magnum/Trade/MeshData.h"

Sphere::Sphere(std::string name, Object3D* parent, float scale, const float mass, btDynamicsWorld& bWorld):
GameObject(std::move(name)), _collisionShape(btSphereShape{scale}) {
    _mass = mass;
    _scale = scale;
    _mesh = MeshTools::compile(Primitives::uvSphereSolid(16, 32));
    _mesh.addVertexBufferInstanced(GL::Buffer{}, 1, 0,
        Shaders::PhongGL::TransformationMatrix{},
        Shaders::PhongGL::NormalMatrix{},
        Shaders::PhongGL::Color3{});
    this->_rigidBody = new RigidBody{parent, _mass, &_collisionShape, bWorld};
}

Sphere::Sphere(Object3D* parent, const float scale, const float mass, btDynamicsWorld& bWorld):
GameObject("Sphere"), _collisionShape(btSphereShape{scale}) {
    _mass = mass;
    _scale = scale;
    this->_rigidBody = new RigidBody{parent, _mass, &_collisionShape, bWorld};
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

