//
// Created by User on 09/04/2025.

#include "entities/primitives/Player.h"


Player::Player(ENetPeer *peer, Engine* app, Object3D *parent) :
    GameObject("Player " + std::to_string(5), 0), _collisionShape(btVector3(0.2f, 0.2f, 0.2f))
{
    _peer = peer;
    _app = app;
    _playerID = 5; // Not initialised for now
    _scale = btVector3(0.2f, 0.2f, 0.2f);
    _collisionShape = btBoxShape(_scale);
    _type = CUBE;
    _parent = parent;
}

Player::Player(ENetPeer *peer, Engine* app, Object3D *parent, const uint8_t id) :
    GameObject("Player " + std::to_string(id), 0), _collisionShape(btVector3(0.2f, 0.2f, 0.2f))
{
    _peer = peer;
    _app = app;
    _playerID = id;
    _scale = btVector3(0.2f, 0.2f, 0.2f);
    _collisionShape = btBoxShape(_scale);
    _type = CUBE;
    _parent = parent;

    // Change name if it already exists
    giveDefaultName();

    // Physics
    this->_rigidBody = new RigidBody{parent, _mass, &_collisionShape, _app->getWorld()};

    // Appearances
    switch (id) {
        case 0: _color = Color3::red(); break;
        case 1: _color = Color3::green(); break;
        case 2: _color = Color3::blue(); break;
        case 3: _color = Color3::yellow(); break;
        default : break;
    }
    _drawable = new ColoredDrawable{
        *(_rigidBody), _app->getBoxInstanceData(), _color,
        Matrix4::scaling(Vector3{_scale}), _app->getDrawables()
    };

    // Location
    float x = 17 * pow(-1, id);
    float y = 15;
    float z = 17 * pow(-1, id + id / 2);
    Vector3 position = Vector3(x, y, z);
    _rigidBody->translate(position);

    /*
    // Rotation
    Vector3 up = Vector3::xAxis();
    Matrix4 lookAt = Matrix4::lookAt(position, Vector3{0.0f}, up);
    Quaternion rotation = Quaternion::fromMatrix(lookAt.rotationScaling());
    _rigidBody->rotate(rotation);
    */

    _rigidBody->syncPose();
}

void Player::setColor(const Color3 &color) {
    if (_drawable) {
        delete _drawable;
    }
    _color = color;
    _drawable = new ColoredDrawable{*(_rigidBody), _app->getBoxInstanceData(), color,
        Matrix4::scaling(Vector3{_scale}), _app->getDrawables()};
}

void Player::updateBulletFromData() {
    // Reconstruire le RigidBody
    _collisionShape = btBoxShape{_scale};

    if (_rigidBody == nullptr) {
        _rigidBody = new RigidBody{_parent, _mass, &_collisionShape, _app->getWorld()};
        _rigidBody->translate(Vector3(_location));
        _rigidBody->rotate(Quaternion(_rotation));
        _rigidBody->syncPose();
    }
    _rigidBody->rigidBody().getMotionState()->setWorldTransform(btTransform(btQuaternion(_rotation), _location));

    _rigidBody->rigidBody().setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT| btCollisionObject::CF_NO_CONTACT_RESPONSE);
    _rigidBody->rigidBody().setActivationState(DISABLE_DEACTIVATION);
}


void Player::serialize(std::ostream &ostr) const {
    // id
    ostr.write(reinterpret_cast<const char*>(&_playerID), sizeof(uint8_t)); // Gérer quand on déserialise, faut trouver le bon player
    ostr.write(reinterpret_cast<const char*>(&_score), sizeof(uint16_t));
    ostr.write(reinterpret_cast<const char*>(&_fps), sizeof(uint16_t));
    ostr.write(reinterpret_cast<const char*>(&_ping), sizeof(uint8_t));
    GameObject::serialize(ostr);

    // Serialize Scale
    ostr.write(reinterpret_cast<const char*>(&_scale.x()), sizeof(float));
    ostr.write(reinterpret_cast<const char*>(&_scale.y()), sizeof(float));
    ostr.write(reinterpret_cast<const char*>(&_scale.z()), sizeof(float));
}

void Player::unserialize(std::istream &istr) {
    uint8_t checkID;
    istr.read(reinterpret_cast<char*>(&checkID), sizeof(uint8_t));

    if (_playerID == 5) { // New Client
        _playerID = checkID;
    }
    else if (_playerID != checkID) {
        std::cerr << "Player::unserialize(): invalid id. Current : " << _playerID << ", Serialized : " << checkID << std::endl;
    }

    istr.read(reinterpret_cast<char*>(&_score), sizeof(uint16_t));
    istr.read(reinterpret_cast<char*>(&_fps), sizeof(uint16_t));
    istr.read(reinterpret_cast<char*>(&_ping), sizeof(uint8_t));

    // Useless id for the player
    uint32_t useless_id;
    istr.read(reinterpret_cast<char*>(&useless_id), sizeof(uint32_t));

    // booléen de destruction (ne sert à rien pour le player)
    bool reader;
    istr.read(reinterpret_cast<char*>(&reader), sizeof(bool));

    // Type de l'objet (ne sert à rien non plus)
    ObjectType type;
    istr.read(reinterpret_cast<char*>(&type), sizeof(ObjectType));

    GameObject::unserialize(istr);

    // Unserialize Scale
    float x, y, z;
    istr.read(reinterpret_cast<char*>(&x), sizeof(float));
    istr.read(reinterpret_cast<char*>(&y), sizeof(float));
    istr.read(reinterpret_cast<char*>(&z), sizeof(float));
    _scale = btVector3(x, y, z);
}