//
// Created by Albin Horlaville on 08/04/2025.
//

#include "entities/primitives/Cube.h"
#include <entities/primitives/Sphere.h>
#include <systems/physics/ProjectileManager.h>
#include <enet6/enet.h>
#include <Magnum/ImGuiIntegration/Context.hpp>
#include "systems/levels/Server.h"

#include <systems/network/PackageType.h>

#include "../../../cmake-build-debug/_deps/bullet-src/examples/SharedMemory/plugins/b3PluginAPI.h"

Server::Server(const Arguments &arguments): Engine(arguments) {
    initSimulation();

    /* Create the ground */
    auto *ground = new Cube(this, "Floor", &_scene, {5.0f, 0.5f, 5.0f}, 0.f, 0xffffff_rgbf);
    addObject(ground);

    /* Create boxes with random colors */
    Deg hue = 42.0_degf;
    for (Int i = 0; i != 3; ++i) {
        for (Int j = 0; j != 3; ++j) {
            for (Int k = 0; k != 3; ++k) {
                Color3 color = Color3::fromHsv({hue += 137.5_degf, 0.75f, 0.9f});
                auto *o = new Cube(this, &_scene, {0.5f, 0.5f, 0.5f}, 3.f, color);
                o->_rigidBody->translate({i - 2.0f, j + 4.0f, k - 2.0f});
                o->_rigidBody->syncPose();
                addObject(o);
            }
        }
    }

    // RX initialisation
    initENet6();
}

Server::~Server() {
    Engine::~Engine();
    enet_host_destroy(_server);
}

void Server::initENet6() {
    if (enet_initialize() != 0) {
        std::cerr << "Error during ENet6 initialization." << std::endl;
        return;
    }

    atexit(enet_deinitialize); // Nettoyage automatique à la fin du programme

    ENetAddress address;
    enet_address_build_any(&address, ENET_ADDRESS_TYPE_IPV6);
    address.port = 5555;

    _server = enet_host_create (ENET_ADDRESS_TYPE_ANY, /* either has to match address->type or be ENET_ADDRESS_TYPE_ANY to dual stack the socket */
                               & address /* the address to bind the server host to */,
                               4      /* allow up to 4     clients and/or outgoing connections */,
                               2      /* allow up to 2 channels to be used, 0 and 1 */,
                               0      /* assume any amount of incoming bandwidth */,
                               0      /* assume any amount of outgoing bandwidth */);

    if (_server == nullptr) {
        std::cerr << "Error during Enet6 client creation." << std::endl;
        return;
    }

    std::cout << "Server listening port : " << address.port << "..." << std::endl;
}

void Server::tickEvent() {
    ++_frame;
    tickMovments();
    cleanWorld();
    networkUpdate();

    // Simulation physique
    _pWorld->_bWorld->stepSimulation(_timeline.previousFrameDuration(), 5);

    // Avance la timeline et redessine
    _timeline.nextFrame();
    redraw();

    if (_players[0])
        std::cout << "Frame :" << _frame  << " Client : " << _players[0]->_currentFrame << std::endl;
}

void Server::networkUpdate() {
    ENetEvent event;
    while (enet_host_service(_server, &event, 1) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                handleConnect(event);
                break;
            }
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
    sendSnapshot();
    /*
    snapshotTimer += deltaTime.get();

    if(snapshotTimer >= 0.1f) {
        sendSnapshot();
        snapshotTimer = 0.0f;
    }
    */
}

void Server::handleConnect(const ENetEvent &event) {
    char ipStr[46]; // INET6_ADDRSTRLEN
    enet_address_get_host_ip(&event.peer->address, ipStr, sizeof(ipStr));
    std::cout << "New client connected on " << ipStr << std::endl;
    event.peer->data = static_cast<void*>(const_cast<char*>("Client connected"));

    // Créer un nouveau player si la limite de 4 clients n'a pas été atteinte.
    for (int i = 0; i < 4; i++) {
        if (_players[i] == nullptr) {
            std::cout << "Creation d'un nouveau player..." << std::endl;
            _players[i] = new Player(event.peer, this, &_scene, i);
            _players[i]->_rigidBody->translate({-5.0f, 4.0f, -5.0f + static_cast<float>(i)});

            // Send ID of the player
            std::ostringstream oss(std::ios::binary);
            PackageType flag = MSG_ASSIGN_ID;
            oss.write(reinterpret_cast<const char*>(&flag), sizeof(flag));
            oss.write(reinterpret_cast<const char*>(&i), sizeof(i));

            std::string data = oss.str();
            ENetPacket* packet = enet_packet_create(data.data(), data.size(), ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(_players[i]->_peer, 1, packet);
            std::cout << "Id " << i << " sent to the client." << std::endl;
            break;
        }
    }
}

void Server::handleReceive(const ENetEvent &event) {
    size_t payloadSize = event.packet->dataLength;
    const char* payload = reinterpret_cast<const char*>(event.packet->data);
    std::istringstream iss(std::string(payload, payloadSize), std::ios::binary);
    PackageType type;
    iss.read(reinterpret_cast<char*>(&type), sizeof(type));

    switch (type) {
        case MSG_WORLD_ACK: {
            uint8_t id_client;
            iss.read(reinterpret_cast<char*>(&id_client), sizeof(id_client));
            uint64_t frame;
            iss.read(reinterpret_cast<char*>(&frame), sizeof(frame));
            _players[id_client]->_currentFrame = frame;
            break;
        }
        default: break;
    }
}

void Server::handleDisconnect(const ENetEvent &event) {
    std::cout << "Client disconnected." << std::endl;
    event.peer->data = nullptr;
}

void Server::sendSnapshot() {
    std::ostringstream oss(std::ios::binary);

    // Mettre le flag
    PackageType flag = MSG_WORLD_SYNC;
    oss.write(reinterpret_cast<const char*>(&flag), sizeof(flag));

    serialize(oss); // Ton appel à serialize(std::ostream&)

    std::string data = oss.str();
    ENetPacket* packet = enet_packet_create(
        data.data(), data.size(),
        ENET_PACKET_FLAG_RELIABLE
    );

    for (auto player : _players) {
        if (player) {
            enet_peer_send(player->_peer, 1, packet);
        }
    }
}

void Server::serialize(std::ostream &ostr) const {
    // Serialize frame number
    ostr.write(reinterpret_cast<const char*>(&_frame), sizeof(uint64_t));

    // Sérialiser les players
    /*
    for (int i = 0; i < 4; i++) {
        if (_players[i]) {
            _players[i]->serialize(ostr);
        }
    }
    */
    // On sérialise le nombre d'objet que l'on sérialise
    uint16_t size_objects = _objects.size();
    ostr.write(reinterpret_cast<const char*>(&size_objects), sizeof(uint16_t));
    for (auto pair : _objects) {
        pair.second->serialize(ostr);
    }
}

void Server::unserialize(std::istream &istr) {
    // Unserialize frame number
    istr.read(reinterpret_cast<char*>(&_frame), sizeof(uint64_t));

    // Players
    /*
    for (int i = 0; i < 4; i++) {
        if (_players[i] == nullptr) {
            _players[i] = new Player(5, nullptr, this, &_scene);
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
                    addObject(cube);
                    break;
                }
                case SPHERE: {
                    auto* sphere = new Sphere(this, &_scene);
                    sphere->unserialize(istr);
                    sphere->updateBulletFromData();
                    addObject(sphere);
                    break;
                }
            }
        }
    }
}
