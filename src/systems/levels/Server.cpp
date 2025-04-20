//
// Created by Albin Horlaville on 08/04/2025.
//

#include "entities/primitives/Cube.h"
#include <entities/primitives/Sphere.h>
#include <systems/physics/ProjectileManager.h>
#include <enet6/enet.h>
#include <Magnum/ImGuiIntegration/Context.hpp>
#include "systems/levels/Server.h"

#include <sys/stat.h>
#include <systems/network/PackageType.h>

#include "../../../cmake-build-debug/_deps/bullet-src/examples/SharedMemory/plugins/b3PluginAPI.h"

Server::Server(const Arguments &arguments): Engine(arguments) {
    initSimulation();

    /* Camera setup */
    (*(_cameraRig = new Object3D{&_scene}))
        .translate(Vector3::yAxis(3.0f))
        .rotateY(40.0_degf);
    (*(_cameraObject = new Object3D{_cameraRig}))
        .translate(Vector3::zAxis(20.0f))
        .rotateX(-25.0_degf);
    (_camera = new SceneGraph::Camera3D(*_cameraObject))
        ->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
        .setProjectionMatrix(Matrix4::perspectiveProjection(35.0_degf, 1.0f, 0.001f, 99.0f))
        .setViewport(GL::defaultFramebuffer.viewport().size());

    /* Create the ground */
    auto *ground = new Cube(this, "Floor", &_scene, {5.0f, 0.5f, 5.0f}, 0.f, 0xffffff_rgbf);
    addObject(ground);

    /* Create boxes with random colors */
    Deg hue = 42.0_degf;
    for (Int i = 0; i != 3; ++i) {
        for (Int j = 0; j != 3; ++j) {
            for (Int k = 0; k != 3; ++k) {
                Color3 color = Color3(1.0f, 1.0f, 1.0f);
                auto *o = new Cube(this, &_scene, {0.5f, 0.5f, 0.5f}, 3.f, color);
                o->_rigidBody->translate({i - 2.0f, j + 2.0f, k - 2.0f});
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
    handleCollision();
    networkUpdate();

    // Simulation physique
    _pWorld->_bWorld->stepSimulation(_timeline.previousFrameDuration(), 5);

    // Avance la timeline et redessine
    _timeline.nextFrame();
    redraw();
}

void Server::handleCollision() {
    CollisionCallback callback;
    callback.onCollision = [&](btRigidBody* a, btRigidBody* b) {
        auto* gameObjA = static_cast<GameObject*>(a->getUserPointer());
        auto* gameObjB = static_cast<GameObject*>(b->getUserPointer());
        if (auto* sphere = dynamic_cast<Sphere*>(gameObjB); sphere != nullptr) {
            return;
        }
        if (gameObjA->_mass == 0 || gameObjB->_mass == 0) {
            return; // Static objects are ignored
        }
        switch(gameObjA->_owner) {
            case 0:
                gameObjB->setColor(Color3::red());
                gameObjB->_owner = 0;
            break;
            case 1:
                gameObjB->setColor(Color3::green());
                gameObjB->_owner = 1;
            break;
            case 2:
                gameObjB->setColor(Color3::blue());
                gameObjB->_owner = 2;
            break;
            case 3:
                gameObjB->setColor(Color3::yellow());
                gameObjB->_owner = 3;
            break;
        }
    };
    for (auto& [id, obj] : _objects) {
        if (obj->_mass == 0)
            continue;
        btRigidBody* body = obj->_rigidBody->_bRigidBody.get();
        _pWorld->_bWorld->contactTest(body, callback);
    }
}


void Server::cleanWorld() {
    _pWorld->cleanWorld();

    for (auto player : _players) {
        if (player) {
            player->updateDataFromBullet();
        }
    }

    // Remove object if their _rigidBody have been destroyed
    for (auto it = _objects.begin(); it != _objects.end(); ) {
        GameObject* object = it->second;
        object->updateDataFromBullet();
        if (object && object->_rigidBody && object->_rigidBody->_bRigidBody) {
            if (object->_rigidBody->_bRigidBody->getWorldTransform().getOrigin().length() > 99.0f) {
                if (_sceneTreeUI->_selectedObject == object) {
                    _sceneTreeUI->_selectedObject = nullptr;
                }
                if (Cube* cube = dynamic_cast<Cube*>(object); cube != nullptr && cube->_owner != -1) {
                    _players[cube->_owner]->_score++;
                }
                _destroyedObjects.push_back(new DestroyedObject{object->_id, _frame});
                _linkingContext.Unregister(object);
                it = _objects.erase(it);
                continue;
            }
        }
        ++it;
    }

    // Clean list of destroyed objects
    for (auto it = _destroyedObjects.begin(); it != _destroyedObjects.end(); ) {
        DestroyedObject* obj = *it;

        bool everyClientHasRemovedIt = true;
        for (auto player : _players) {
            if (player && player->_currentFrame < obj->frame) {
                everyClientHasRemovedIt = false;
                break;
            }
        }

        if (everyClientHasRemovedIt) {
            delete obj;
            it = _destroyedObjects.erase(it);
        } else {
            ++it;
        }
    }
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
            iss.read(reinterpret_cast<char*>(&id_client), sizeof(uint8_t));
            uint64_t frame;
            iss.read(reinterpret_cast<char*>(&frame), sizeof(uint64_t));
            _players[id_client]->_currentFrame = frame;
            break;
        }
        case MSG_INPUTS: {
            uint8_t id_client;
            iss.read(reinterpret_cast<char*>(&id_client), sizeof(uint8_t));
            Input inputs = Input::None;
            iss.read(reinterpret_cast<char*>(&inputs), sizeof(uint8_t));
            if (inputs == Input::None) {
                break;
            }
            const float moveSpeed = 10.f;
            const float deltaTime = _timeline.previousFrameDuration();
            Vector3 move;
            Player* player = _players[id_client];
            if (hasFlag(inputs, Input::MoveForward))  move += Vector3::zAxis(-moveSpeed * deltaTime);
            if (hasFlag(inputs, Input::MoveBackward)) move += Vector3::zAxis(moveSpeed * deltaTime);
            if (hasFlag(inputs, Input::MoveLeft))     move += Vector3::xAxis(-moveSpeed * deltaTime);
            if (hasFlag(inputs, Input::MoveRight))    move += Vector3::xAxis(moveSpeed * deltaTime);
            if (hasFlag(inputs, Input::MoveUp))       move += Vector3::yAxis(moveSpeed * deltaTime);
            if (hasFlag(inputs, Input::MoveDown))     move += Vector3::yAxis(-moveSpeed * deltaTime);
            player->_rigidBody->translate(move);
            player->_rigidBody->syncPose();
            if (hasFlag(inputs, Input::Shoot)) {
                float x, y, z;
                iss.read(reinterpret_cast<char*>(&x), sizeof(float));
                iss.read(reinterpret_cast<char*>(&y), sizeof(float));
                iss.read(reinterpret_cast<char*>(&z), sizeof(float));
                btVector3 direction(x, y, z);
                iss.read(reinterpret_cast<char*>(&x), sizeof(float));
                iss.read(reinterpret_cast<char*>(&y), sizeof(float));
                iss.read(reinterpret_cast<char*>(&z), sizeof(float));
                Vector3 location(x, y, z);
                GameObject* projectile = _pProjectileManager->Shoot(this, &_scene, location, direction);
                projectile->_rigidBody->_bRigidBody->setUserPointer(projectile);
                projectile->_owner = id_client;
                Color3 color;
                switch (id_client) {
                    case 0: color = Color3::red(); break;
                    case 1: color = Color3::green(); break;
                    case 2: color = Color3::blue(); break;
                    case 3: color = Color3::yellow(); break;
                    default : break;
                }
                projectile->setColor(color);
                projectile->setMass(1000);
                addObject(projectile);
            }
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
    uint8_t number_of_players = 0;
    for (auto player : _players) {
        if (player) {
            number_of_players++;
        }
    }
    ostr.write(reinterpret_cast<const char*>(&number_of_players), sizeof(uint8_t));
    for (int i = 0; i < 4; i++) {
        if (_players[i]) {
            _players[i]->serialize(ostr);
        }
    }

    // On sérialise le nombre d'objet que l'on sérialise
    uint16_t size_objects = _objects.size();
    uint16_t size_destroyed_objects = _destroyedObjects.size();
    uint16_t size_all_objects = size_objects + size_destroyed_objects;
    ostr.write(reinterpret_cast<const char*>(&size_all_objects), sizeof(uint16_t));
    for (auto pair : _objects) {
        pair.second->serialize(ostr);
    }
    // id and true meaning that object has been destroyed on server
    for (auto removed_object : _destroyedObjects) {
        ostr.write(reinterpret_cast<const char*>(&removed_object->id), sizeof(uint32_t));
        bool to_be_removed = true;
        ostr.write(reinterpret_cast<const char*>(&to_be_removed), sizeof(bool));
    }
}

void Server::unserialize(std::istream &istr) {
    /*
    // Unserialize frame number
    istr.read(reinterpret_cast<char*>(&_frame), sizeof(uint64_t));

    // Players
    for (int i = 0; i < 4; i++) {
        if (_players[i] == nullptr) {
            _players[i] = new Player(5, nullptr, this, &_scene);
        }
        _players[i]->unserialize(istr);
    }

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
    */
}
