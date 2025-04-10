//
// Created by User on 09/04/2025.

#include "entities/primitives/Player.h"


Player::Player(uint32_t id, ENetPeer *peer, Engine* app, Object3D *parent) :
    GameObject("Player " + std::to_string(id), 0), _collisionShape(btVector3(0.2f, 0.2f, 0.2f))
{
    _peer = peer;
    _app = app;
    _playerID = id;
    _scale = btVector3(0.2f, 0.2f, 0.2f);
    _collisionShape = btBoxShape(_scale);
    _type = CUBE;

    // Change name if it already exists
    giveDefaultName();

    // Physics
    this->_rigidBody = new RigidBody{parent, _mass, &_collisionShape, _app->getWorld()};

    // Appearances
    Color3 color = Color3(0.5, 0.5, 1);
    new ColoredDrawable{
        *(_rigidBody), _app->getBoxInstanceData(), color,
        Matrix4::scaling(Vector3{_scale}), _app->getDrawables()
    };
}

void Player::serialize(std::ostream &ostr) const {
    // id
    ostr.write(reinterpret_cast<const char*>(&_playerID), sizeof(uint8_t)); // Gérer quand on déserialise, faut trouver le bon player

    GameObject::serialize(ostr);

    // Serialize Scale
    ostr.write(reinterpret_cast<const char*>(&_scale.x()), sizeof(float));
    ostr.write(reinterpret_cast<const char*>(&_scale.y()), sizeof(float));
    ostr.write(reinterpret_cast<const char*>(&_scale.z()), sizeof(float));
}

void Player::unserialize(std::istream &istr) {
    int checkID;
    istr.read(reinterpret_cast<char*>(&checkID), sizeof(uint8_t));

    if (_playerID != checkID) {
        std::cerr << "Player::unserialize(): invalid id" << std::endl;
    }

    GameObject::unserialize(istr);

    // Unserialize Scale
    float x, y, z;
    istr.read(reinterpret_cast<char*>(&x), sizeof(float));
    istr.read(reinterpret_cast<char*>(&y), sizeof(float));
    istr.read(reinterpret_cast<char*>(&z), sizeof(float));
    _scale = btVector3(x, y, z);

    // Reconstruire le RigidBody
    // Physics
    _collisionShape = btBoxShape{_scale};
    this->_rigidBody = new RigidBody{_parent, _mass, &_collisionShape, _app->getWorld()};
    _rigidBody->rigidBody().activate();

    _rigidBody->translate(Vector3(_location));
    _rigidBody->rotate(Quaternion(_rotation));
    _rigidBody->syncPose();

    _rigidBody->rigidBody().setLinearVelocity(_linearVelocity);
    _rigidBody->rigidBody().setAngularVelocity(_angularVelocity);

    _rigidBody->rigidBody().setActivationState(ACTIVE_TAG);
    _rigidBody->rigidBody().activate(true);

    // Appearance
    Color3 color = Color3(1, 1, 1);
    new ColoredDrawable{*(_rigidBody), _app->getBoxInstanceData(), color,
    Matrix4::scaling(Vector3{_scale}), _app->getDrawables()};
}