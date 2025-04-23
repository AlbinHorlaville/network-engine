//
// Created by User on 14/03/2025.
//

#include <utility>

#include "entities/primitives/Cube.h"
#include "Magnum/Primitives/Cube.h"

Cube::Cube(Engine* app, std::string name, Object3D *parent, const btVector3 scale, float mass, const Color3& color):
    GameObject(std::move(name), mass), _collisionShape(btBoxShape{scale}) {
    _app = app;
    _scale = scale;
    _type = CUBE;

    // Change name if it already exists
    giveDefaultName();

    // Physics
    this->_rigidBody = new RigidBody{parent, _mass, &_collisionShape, _app->getWorld()};
    _rigidBody->_bRigidBody->setUserPointer(static_cast<void*>(this));

    // Appearances
    Cube::setColor(color);
}

Cube::Cube(Engine* app, Object3D* parent, const btVector3 scale, const float mass, const Color3& color):
    GameObject("Cube", mass), _collisionShape(btBoxShape{scale}) {
    _app = app;
    _scale = scale;
    _type = CUBE;

    // Change name if it already exists
    giveDefaultName();

    // Physics
    this->_rigidBody = new RigidBody{parent, _mass, &_collisionShape, _app->getWorld()};
    _rigidBody->_bRigidBody->setUserPointer(static_cast<void*>(this));
    // Appearance
    Cube::setColor(color);
}

Cube::Cube(Engine* app, Object3D* parent):
    GameObject("None", 1), _scale(btVector3(1, 1, 1)), _collisionShape(btBoxShape{_scale}) {
    _app = app;
    _type = CUBE;
    _parent = parent;

    // Change name if it already exists
    giveDefaultName();
}

void Cube::setScale(btVector3 newScale) {
    _scale = newScale;

    // Créer un nouveau shape avec make_unique
    _collisionShape = btBoxShape{_scale};
    _rigidBody->rigidBody().setCollisionShape(&_collisionShape);

    // Recalculer l'inertie si le corps est dynamique
    const btScalar mass = _rigidBody->rigidBody().getInvMass() == 0 ? 0 : static_cast<btScalar>(1.0) / _rigidBody->rigidBody().getInvMass();
    btVector3 inertia(0, 0, 0);
    if (mass > 0.f) {
        _collisionShape.calculateLocalInertia(mass, inertia);
    }
    _rigidBody->rigidBody().setMassProps(mass, inertia);
    _rigidBody->rigidBody().updateInertiaTensor();

    // Mettre à jour la matrice de transformation graphique
    _rigidBody->setTransformation(Matrix4::scaling(Vector3{_scale}));

    // Réactiver le corps rigide
    _rigidBody->rigidBody().activate();
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

    // Désactiver le rigid body avant modification
    _rigidBody->rigidBody().setActivationState(DISABLE_SIMULATION);

    // Appliquer la nouvelle masse et l'inertie
    _rigidBody->rigidBody().setMassProps(_mass, inertia);

    // Réactiver le rigid body
    _rigidBody->rigidBody().setActivationState(ACTIVE_TAG);
}

void Cube::setColor(const Color3 &color) {
    if (_drawable) {
        delete _drawable;
    }
    _color = color;
    _drawable = new ColoredDrawable{*(_rigidBody), _app->getBoxInstanceData(), color,
        Matrix4::scaling(Vector3{_scale}), _app->getDrawables()};
}

void Cube::updateBulletFromData() {
    // Reconstruire le RigidBody
    _collisionShape = btBoxShape{_scale};

    if (_rigidBody == nullptr) {
        _rigidBody = new RigidBody{_parent, _mass, &_collisionShape, _app->getWorld()};
        _rigidBody->translate(Vector3(_location));
        _rigidBody->rotate(Quaternion(_rotation));
        _rigidBody->syncPose();
    }
    _rigidBody->rigidBody().getMotionState()->setWorldTransform(btTransform(btQuaternion(_rotation), _location));

    _rigidBody->rigidBody().setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT);
    _rigidBody->rigidBody().setActivationState(DISABLE_DEACTIVATION);

    // Appearance
    setColor(_color);
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
}
