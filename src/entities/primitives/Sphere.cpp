//
// Created by User on 14/03/2025.
//

#include "entities/primitives/Sphere.h"

#include <set>

Sphere::Sphere(Engine* app, std::string name, Object3D *parent, float scale, float mass, const Color3& color):
GameObject(std::move(name), mass), _collisionShape(btSphereShape{scale}) {
    _app = app;
    _scale = scale;
    _type = SPHERE;

    // Change name if it already exists
    giveDefaultName();

    // Physics
    this->_rigidBody = new RigidBody{parent, _mass, &_collisionShape, _app->getWorld()};

    // Appearance
    Sphere::setColor(color);
}

Sphere::Sphere(Engine* app, Object3D* parent, const float scale, const float mass, const Color3& color):
    GameObject("Sphere", mass), _collisionShape(btSphereShape{scale}) {
    _app = app;
    _scale = scale;
    _type = SPHERE;

    // Change name if it already exists
    giveDefaultName();

    // Physics
    this->_rigidBody = new RigidBody{parent, _mass, &_collisionShape, _app->getWorld()};

    // Appearance
    Sphere::setColor(color);
}

Sphere::Sphere(Engine* app, Object3D* parent):
    GameObject("None", 1), _scale(1), _collisionShape(btSphereShape{_scale}) {
    _app = app;
    _type = CUBE;
    _parent = parent;

    // Change name if it already exists
    giveDefaultName();
}

void Sphere::setScale(const float newScale) {
    _scale = newScale;

    // Créer un nouveau shape avec make_unique
    _collisionShape = btSphereShape{_scale};
    _rigidBody->rigidBody().setCollisionShape(&_collisionShape);

    // Recalculer l'inertie si le corps est dynamique
    btScalar mass = _rigidBody->rigidBody().getInvMass() == 0 ? 0 : 1.0 / _rigidBody->rigidBody().getInvMass();
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

void Sphere::setMass(const float mass) {
    _mass = mass;

    // Compute the inertia tensor for a cube
    // Assuming the cube is aligned along the axes and centered at the origin.
    const float sideLength = _scale * 2.0f;  // _scale is the half side length
    const float I = mass * sideLength * sideLength / 6.0f;  // Inertia tensor for a cube

    // Create the inertia tensor (diagonal)
    const btVector3 inertia(I, I, I);  // For a cube, the inertia tensor is the same on all axes

    // Désactiver le rigid body avant modification
    _rigidBody->rigidBody().setActivationState(DISABLE_SIMULATION);

    // Appliquer la nouvelle masse et l'inertie
    _rigidBody->rigidBody().setMassProps(_mass, inertia);

    // Réactiver le rigid body
    _rigidBody->rigidBody().setActivationState(ACTIVE_TAG);
}

void Sphere::setColor(const Color3 &color) {
    if (_drawable) {
        delete _drawable;
    }
    _color = color;
    _drawable = new ColoredDrawable{*(_rigidBody), _app->getSphereInstanceData(), color,
        Matrix4::scaling(Vector3{_scale}), _app->getDrawables()};
}

void Sphere::updateBulletFromData() {
    // Reconstruire le RigidBody
    _collisionShape = btSphereShape{_scale};

    if (_rigidBody == nullptr) {
        _rigidBody = new RigidBody{_parent, _mass, &_collisionShape, _app->getWorld()};
        _rigidBody->translate(Vector3(_location));
        _rigidBody->rotate(Quaternion(_rotation));
        _rigidBody->syncPose();
    }
    _rigidBody->rigidBody().getMotionState()->setWorldTransform(btTransform(btQuaternion(_rotation), _location));

    _rigidBody->rigidBody().setLinearVelocity(_linearVelocity);
    _rigidBody->rigidBody().setAngularVelocity(_angularVelocity);
    _rigidBody->rigidBody().setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT);
    _rigidBody->rigidBody().setActivationState(DISABLE_DEACTIVATION);

    _rigidBody->rigidBody().setLinearVelocity(_linearVelocity);
    _rigidBody->rigidBody().setAngularVelocity(_angularVelocity);

    // Appearance
    setColor(_color);
}

void Sphere::serialize(std::ostream &ostr) const {
    GameObject::serialize(ostr);

    // Serialize Scale
    ostr.write(reinterpret_cast<const char*>(&_scale), sizeof(float));
}

void Sphere::unserialize(std::istream &istr) {
    GameObject::unserialize(istr);

    // Unserialize Scale
    istr.read(reinterpret_cast<char*>(&_scale), sizeof(float));
}
