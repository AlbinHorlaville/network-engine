//
// Created by User on 08/04/2025.
//

#include "systems/physics/PhysicsWorld.h"
#include "entities/primitives/Cube.h"
#include <enet6/enet.h>
#include "systems/levels/Client.h"
#include "systems/network/PackageType.h"
#include <entities/primitives/Sphere.h>

Client::Client(const Arguments &arguments): Engine(arguments) {
    initSimulation();

    // RX initialisation
    initENet6();
}

Client::~Client() {
    Engine::~Engine();
    enet_peer_reset(_peer);
    enet_host_destroy(_client);
}

void Client::initENet6() {
    if (enet_initialize() != 0) {
        std::cerr << "Error during ENet6 initialization." << std::endl;
        return;
    }

     _client = enet_host_create(ENET_ADDRESS_TYPE_IPV6, nullptr, 1, 2, 0, 0);
    if (_client == nullptr) {
        std::cerr << "Error during Enet6 client creation." << std::endl;
        return;
    }

    ENetAddress address;
    enet_address_set_host(&address, ENET_ADDRESS_TYPE_IPV6, "::1"); // Adresse du serveur
    address.port = 5555;

    _peer = enet_host_connect(_client, &address, 2, 0);
    if (_peer == nullptr) {
        std::cerr << "Impossible to connect to the server" << std::endl;
        enet_host_destroy(_client);
        return;
    }

    ENetEvent event;
    if (enet_host_service(_client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        std::cout << "Connected to the server." << std::endl;
    } else {
        std::cerr << "Connection to the server failed." << std::endl;
        enet_peer_reset(_peer);
        enet_host_destroy(_client);
    }
}

void Client::tickEvent() {
    networkUpdate();
    tickMovments();

    // Simulation physique
    _pWorld->_bWorld->stepSimulation(_timeline.previousFrameDuration(), 5);

    // Avance la timeline et redessine
    _timeline.nextFrame();
    redraw();
}

void Client::networkUpdate() {
    ENetEvent event;
    while (enet_host_service(_client, &event, 1) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE: {
                handleReceive(event);
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT: {
                handleDisconnect(event);
                break;
            }
            default: break;
        }
    }
    enet_packet_destroy(event.packet);
}

void Client::handleReceive(const ENetEvent &event) {
    size_t payloadSize = event.packet->dataLength;
    const char* payload = reinterpret_cast<const char*>(event.packet->data);
    std::istringstream iss(std::string(payload, payloadSize), std::ios::binary);
    PackageType type;
    iss.read(reinterpret_cast<char*>(&type), sizeof(type));

    switch (type) {
        case MSG_ASSIGN_ID: {
            uint8_t id;
            iss.read(reinterpret_cast<char*>(&id), sizeof(id));
            std::cout << "ID recu du serveur : " << static_cast<int>(id) << std::endl;

            // Tu peux sauvegarder cet ID dans une variable membre, si besoin
            _id = id;
            break;
        }
        case MSG_WORLD_SYNC: {
            unserialize(iss);
            sendSnapshotACK(event.peer);
            break;
        }
        default: break;
    }
}

void Client::handleDisconnect(const ENetEvent &event) {
    std::cout << "Disconnected from the server." << std::endl;
}

void Client::sendSnapshotACK(ENetPeer *peer) {
    std::ostringstream oss(std::ios::binary);

    // Mettre le flag
    PackageType flag = MSG_WORLD_ACK;
    oss.write(reinterpret_cast<const char*>(&flag), sizeof(PackageType));
    oss.write(reinterpret_cast<const char*>(&_id), sizeof(uint8_t));
    oss.write(reinterpret_cast<const char*>(&_frame), sizeof(uint64_t));

    std::string data = oss.str();
    ENetPacket* packet = enet_packet_create(
        data.data(), data.size(),
        ENET_PACKET_FLAG_RELIABLE
    );

    enet_peer_send(peer, 1, packet);
}


void Client::pointerPressEvent(PointerEvent &event) {
    if(_imgui.handlePointerPressEvent(event)) return;
}

void Client::keyPressEvent(KeyEvent &event) {

}

void Client::serialize(std::ostream &ostr) const {
    // Serialize frame number
    ostr.write(reinterpret_cast<const char*>(&_frame), sizeof(uint64_t));

    // Sérialiser les players
    for (int i = 0; i < 4; i++) {
        if (_players[i]) {
            _players[i]->serialize(ostr);
        }
    }
    // On sérialise le nombre d'objet que l'on sérialise
    uint16_t size_objects = _objects.size();
    ostr.write(reinterpret_cast<const char*>(&size_objects), sizeof(uint16_t));
    for (auto pair : _objects) {
        pair.second->serialize(ostr);
    }
}

void Client::unserialize(std::istream &istr) {
    // Unserialize frame number
    istr.read(reinterpret_cast<char*>(&_frame), sizeof(uint64_t));

    // Players
    /*
    for (int i = 0; i < 4; i++) {
        if (_players[i] == nullptr) {
            _players[i] = new Player(nullptr, this, &_scene);
        }
        _players[i]->unserialize(istr);
    }
    */
    // Objets
    uint16_t size_objects;
    istr.read(reinterpret_cast<char*>(&size_objects), sizeof(uint16_t));
    for (uint16_t i = 0; i < size_objects; i++) {
        // Désérialiser l'ID d'objet.
        uint32_t id;
        istr.read(reinterpret_cast<char*>(&id), sizeof(uint32_t));

        // Désérialiser le booléen de destruction
        bool has_been_destroyed;
        istr.read(reinterpret_cast<char*>(&has_been_destroyed), sizeof(bool));
        if (has_been_destroyed) {
            GameObject* object = _linkingContext.GetLocalObject(id);
            if (object) {
                _linkingContext.Unregister(object);
                if (_sceneTreeUI->_selectedObject == object) {
                    _sceneTreeUI->_selectedObject = nullptr;
                }
                _objects.erase(object->_name);
                _linkingContext.Unregister(object);
            }
            continue;
        }

        GameObject* obj = _linkingContext.GetLocalObject(id);
        if (obj) { // L'objet est trouvé
            ObjectType type;
            istr.read(reinterpret_cast<char*>(&type), sizeof(ObjectType));
            obj->unserialize(istr);
            obj->updateBulletFromData();
        }
        else { // L'objet doit être créé
            ObjectType type;
            istr.read(reinterpret_cast<char*>(&type), sizeof(ObjectType));
            switch(type) {
                case CUBE: {
                    auto cube = new Cube(this, &_scene);
                    cube->unserialize(istr);
                    cube->updateBulletFromData();
                    addObject(cube, id);
                    break;
                }
                case SPHERE: {
                    auto* sphere = new Sphere(this, &_scene);
                    sphere->unserialize(istr);
                    sphere->updateBulletFromData();
                    addObject(sphere, id);
                    break;
                }
            }
        }
    }
}
