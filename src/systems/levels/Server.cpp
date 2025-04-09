//
// Created by Albin Horlaville on 08/04/2025.
//

#include "entities/primitives/Cube.h"
#include <entities/primitives/Sphere.h>
#include <systems/physics/ProjectileManager.h>
#include <enet6/enet.h>
#include <Magnum/ImGuiIntegration/Context.hpp>
#include "systems/levels/Server.h"

Server::Server(const Arguments &arguments): Engine(arguments) {
    initSimulation();

    /* Create the ground */
    auto *ground = new Cube(this, "Floor", &_scene, {5.0f, 0.5f, 5.0f}, 0.f, 0xffffff_rgbf);
    _objects[ground->_name] = ground;

    /* Create boxes with random colors */
    Deg hue = 42.0_degf;
    for (Int i = 0; i != 5; ++i) {
        for (Int j = 0; j != 5; ++j) {
            for (Int k = 0; k != 5; ++k) {
                Color3 color = Color3::fromHsv({hue += 137.5_degf, 0.75f, 0.9f});
                auto *o = new Cube(this, &_scene, {0.5f, 0.5f, 0.5f}, 3.f, color);
                _objects[o->_name] = o;
                o->_rigidBody->translate({i - 2.0f, j + 4.0f, k - 2.0f});
                o->_rigidBody->syncPose();
                // Register o in the Linking Context
                _linkingContext.Register(o);
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
    networkUpdate();
    tickMovments();
    cleanWorld();

    // Simulation physique
    _pWorld->_bWorld->stepSimulation(_timeline.previousFrameDuration(), 5);

    // Avance la timeline et redessine
    _timeline.nextFrame();
    redraw();
}

void Server::networkUpdate() {
    ENetEvent event;
    while (enet_host_service(_server, &event, 1) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                char ipStr[46]; // INET6_ADDRSTRLEN
            enet_address_get_host_ip(&event.peer->address, ipStr, sizeof(ipStr));
            std::cout << "New client connected on " << ipStr << std::endl;
            event.peer->data = static_cast<void*>(const_cast<char*>("Client connected"));
            break;

            case ENET_EVENT_TYPE_RECEIVE:
                std::cout << "Package received on canal " << event.channelID
                          << " : " << reinterpret_cast<char *>(event.packet->data) << std::endl;

            // Réponse (echo)
            enet_peer_send(event.peer, event.channelID, event.packet);
            enet_host_flush(_server); // Envoi immédiat

            break;

            case ENET_EVENT_TYPE_DISCONNECT:
                std::cout << "Client disconnected." << std::endl;
            event.peer->data = nullptr;
            break;

            default:
                break;
        }
    }
}

void Server::keyPressEvent(KeyEvent &) {} // Vide car le server n'a pas besoin d'input

void Server::pointerPressEvent(PointerEvent &) {} // Vide car le server n'a pas besoin d'input


