//
// Created by User on 14/03/2025.
//

#include "entities/GameObject.h"

#include "../../cmake-build-debug/_deps/bullet-src/examples/ThirdPartyLibs/clsocket/src/Host.h"
#include "Corrade/Utility/String.h"


void GameObject::giveDefaultName() {

    if (!_app->getObjects().contains(_name)) {
        return;
    }
    size_t i = _name.size();

    // Trouver le début du nombre à la fin (si existant)
    while (i > 0 && std::isdigit(_name[i - 1])) {
        --i;
    }

    std::string base = _name.substr(0, i); // Partie non numérique
    int number = (i < _name.size()) ? std::stoi(_name.substr(i)) + 1 : 1; // Incrément ou 1 si absent

    _name = base + std::to_string(number);
    giveDefaultName();
}

void GameObject::updateDataFromBullet() {
    btTransform transform =  _rigidBody->_bRigidBody->getWorldTransform();
    _location = transform.getOrigin();
    _rotation = transform.getRotation();
}

void GameObject::serialize(std::ostream &ostr) const {
    // Serialize the object id
    ostr.write(reinterpret_cast<const char*>(&_id), sizeof(uint32_t));

    // Serialize the type of object (Cube, Sphere, ...)
    ostr.write(reinterpret_cast<const char*>(&_type), sizeof(ObjectType));

    // Serialize the name of this object
    size_t length = _name.size();
    ostr.write(reinterpret_cast<const char*>(&length), sizeof(size_t));
    ostr.write(_name.data(), length);

    // Serialize Location
    ostr.write(reinterpret_cast<const char*>(&_location.x()), sizeof(float));
    ostr.write(reinterpret_cast<const char*>(&_location.y()), sizeof(float));
    ostr.write(reinterpret_cast<const char*>(&_location.z()), sizeof(float));

    // Serialize Rotation
    ostr.write(reinterpret_cast<const char*>(&_rotation.x()), sizeof(float));
    ostr.write(reinterpret_cast<const char*>(&_rotation.y()), sizeof(float));
    ostr.write(reinterpret_cast<const char*>(&_rotation.z()), sizeof(float));
    ostr.write(reinterpret_cast<const char*>(&_rotation.w()), sizeof(float));

    // Serialize Mass
    ostr.write(reinterpret_cast<const char*>(&_mass), sizeof(float));
}

void GameObject::unserialize(std::istream &istr) {
    // La désérialisation de l'id se fait avant l'appel de cette méthode

    size_t length;
    istr.read(reinterpret_cast<char*>(&length), sizeof(size_t));
    istr.read(&_name[0], length);  // Lit les caractères du fichier

    // Unserialize Location
    float x, y, z;
    istr.read(reinterpret_cast<char*>(&x), sizeof(float));
    istr.read(reinterpret_cast<char*>(&y), sizeof(float));
    istr.read(reinterpret_cast<char*>(&z), sizeof(float));
    _location = btVector3(x, y, z);

    // Unserialize Rotation
    float w;
    istr.read(reinterpret_cast<char*>(&x), sizeof(float));
    istr.read(reinterpret_cast<char*>(&y), sizeof(float));
    istr.read(reinterpret_cast<char*>(&z), sizeof(float));
    istr.read(reinterpret_cast<char*>(&w), sizeof(float));
    _rotation = btQuaternion(x, y, z, w);

    // Unserialize Mass
    istr.read(reinterpret_cast<char*>(&_mass), sizeof(float));
}
