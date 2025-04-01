//
// Created by User on 14/03/2025.
//

#include <utility>

#include "entities/primitives/Cube.h"
#include "Magnum/Primitives/Cube.h"

Cube::Cube(Level_1* app, std::string name, Object3D *parent, const btVector3 scale, float mass, const Color3& color):
    GameObject(std::move(name), mass), _collisionShape(btBoxShape{scale}) {
    _app = app;
    _scale = scale;
    _type = CUBE;

    // Change name if it already exists
    giveDefaultName();

    // Physics
    this->_rigidBody = new RigidBody{parent, _mass, &_collisionShape, _app->getWorld()};

    // Appearances
    new ColoredDrawable{*(_rigidBody), _app->getBoxInstanceData(), color,
    Matrix4::scaling(Vector3{_scale}), _app->getDrawables()};
}

Cube::Cube(Level_1* app, Object3D* parent, const btVector3 scale, const float mass, const Color3& color):
    GameObject("Cube", mass), _collisionShape(btBoxShape{scale}) {
    _app = app;
    _scale = scale;
    _type = CUBE;

    // Change name if it already exists
    giveDefaultName();

    // Physics
    this->_rigidBody = new RigidBody{parent, _mass, &_collisionShape, _app->getWorld()};

    // Appearance
    new ColoredDrawable{*(_rigidBody), _app->getBoxInstanceData(), color,
    Matrix4::scaling(Vector3{_scale}), _app->getDrawables()};
}

Cube::Cube(Level_1* app, Object3D* parent):
    GameObject("None", 1), _scale(btVector3(1, 1, 1)), _collisionShape(btBoxShape{_scale}) {
    _app = app;
    _type = CUBE;

    // Change name if it already exists
    giveDefaultName();

    // Physics
    this->_rigidBody = new RigidBody{parent, &_collisionShape, _app->getWorld()};

    // Appearance
    Color3 color = Color3(1, 1, 1);
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

void Cube::updateDataFromBullet() {
    GameObject::updateDataFromBullet();
}

void Cube::serialize(std::ostream &ostr) const {
    GameObject::serialize(ostr);

    // Serialize Scale
    ostr.write(reinterpret_cast<const char*>(&_scale.x()), sizeof(float));
    ostr.write(reinterpret_cast<const char*>(&_scale.y()), sizeof(float));
    ostr.write(reinterpret_cast<const char*>(&_scale.z()), sizeof(float));
}

void Cube::unserialize(std::istream &istr) {
    GameObject::unserialize(istr);

    // Unserialize Scale
    float x, y, z;
    istr.read(reinterpret_cast<char*>(&x), sizeof(float));
    istr.read(reinterpret_cast<char*>(&y), sizeof(float));
    istr.read(reinterpret_cast<char*>(&z), sizeof(float));
    _scale = btVector3(x, y, z);

    // Reconstruire le RigidBody
    _rigidBody->createBtRigidBody(_mass);
    setScale(_scale);
    _rigidBody->translate(Vector3(_location));
    _rigidBody->rotate(Quaternion(_rotation));
}
