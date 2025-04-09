//
// Created by User on 08/04/2025.
//

#include "systems/physics/PhysicsWorld.h"
#include "entities/primitives/Cube.h"
#include <enet6/enet.h>
#include "systems/levels/Client.h"

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

        const char* msg = "Hi server !";
        ENetPacket* packet = enet_packet_create(msg, std::strlen(msg) + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(_peer, 0, packet);
    } else {
        std::cerr << "Connection to the server failed." << std::endl;
        enet_peer_reset(_peer);
        enet_host_destroy(_client);
    }
}

void Client::tickEvent() {
    networkUpdate();
    tickMovments();
    cleanWorld();

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
            case ENET_EVENT_TYPE_RECEIVE:
                std::cout << "Message received: " << event.packet->data << std::endl;
            enet_packet_destroy(event.packet);
            break;

            case ENET_EVENT_TYPE_DISCONNECT:
                std::cout << "Disconnected from the server." << std::endl;
            break;

            default:
                break;
        }
    }
}

void Client::pointerPressEvent(PointerEvent &event) {

}

void Client::keyPressEvent(KeyEvent &event) {

}
