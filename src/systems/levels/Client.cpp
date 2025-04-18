//
// Created by User on 08/04/2025.
//

#include "systems/physics/PhysicsWorld.h"
#include "entities/primitives/Cube.h"
#include <enet6/enet.h>
#include "systems/levels/Client.h"
#include "systems/network/PackageType.h"
#include <entities/primitives/Sphere.h>
#include <imgui.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/ImGuiIntegration/Context.hpp>

Client::Client(const Arguments &arguments): Engine(arguments) {
    setSwapInterval(1); // optional vsync
    redraw();
    _cameraObject = new Object3D{&_scene};
    (_camera = new SceneGraph::Camera3D(*_cameraObject))
    ->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
    .setProjectionMatrix(Matrix4::perspectiveProjection(35.0_degf, 1.0f, 0.001f, 99.0f))
    .setViewport(GL::defaultFramebuffer.viewport().size());
    _inputs = new Input(Input::None);
    _pingHandler.init();
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
    glfwPollEvents();

    switch(_state) {
        case (Logged_in) :
            networkUpdate();
            sendInputs();
            // Simulation physique
            _pWorld->_bWorld->stepSimulation(_timeline.previousFrameDuration(), 5);

            // Avance la timeline et redessine
            _timeline.nextFrame();
        break;
        default:
            break;
    }

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
        case MSG_INPUTS_ACK: {
            uint64_t sentTime;
            iss.read(reinterpret_cast<char*>(&sentTime), sizeof(sentTime));

            uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
            std::cout << now - sentTime << std::endl;
            _pingHandler.update(sentTime, now);
            break;
        }
        default: break;
    }
}

void Client::handleDisconnect(const ENetEvent&) {
    std::cout << "Disconnected from the server." << std::endl;
}

void Client::drawEvent() {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);

    switch(_state) {
        case (Logged_in):
            drawGraphics();
            drawImGUI();

            fps_handler.update();
        break;
        case (Not_logged_in) :
            _imgui.newFrame();

            drawLoginWindow();

            // Setup rendering
            GL::Renderer::enable(GL::Renderer::Feature::Blending);
            GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);
            GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);
            GL::Renderer::disable(GL::Renderer::Feature::DepthTest);

            _imgui.drawFrame();  // End ImGui frame (only once!)

            GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
            GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
            GL::Renderer::disable(GL::Renderer::Feature::ScissorTest);
            GL::Renderer::disable(GL::Renderer::Feature::Blending);
        break;
        default:
            break;
    }

    swapBuffers();
    redraw();
};

void Client::drawLoginWindow() {
    ImVec2 windowSize = ImGui::GetMainViewport()->Size;
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always, ImVec2(0.0f, 0.0f)); // Place window in top-left corner
    ImGui::SetNextWindowSize(ImVec2(windowSize.x, windowSize.y)); // Set a dynamic size corresponding to parent window size

    if (ImGui::Begin("Welcome", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        static char usernameLogin[64] = "";
        static char passwordLogin[64] = "";

        if (ImGui::RadioButton("Login", connectTypeOption == 0)) connectTypeOption = 0;
        if (ImGui::RadioButton("Register", connectTypeOption == 1)) connectTypeOption = 1;

        if (ImGui::InputText("Username", usernameLogin, IM_ARRAYSIZE(usernameLogin), ImGuiInputTextFlags_None)) {
            // You can add additional logic here for handling username input
        }
        if (ImGui::InputText("Password", passwordLogin, IM_ARRAYSIZE(passwordLogin), ImGuiInputTextFlags_Password)) {
            // You can add additional logic here for handling password input
        }

        if (ImGui::Button("Connect")) {
            if (connectTypeOption == 0) //Login mode
            {
                initSimulation();
                // RX initialisation
                initENet6();
                _state = Logged_in;
            }
            else //Register mode
            {
                initSimulation();
                // RX initialisation
                initENet6();
                _state = Logged_in;
            }
        }

        ImGui::End();
    }
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

void Client::sendInputs() {
    if (_id > 3) {
        return;
    }
    std::ostringstream oss(std::ios::binary);

    // Mettre le flag
    PackageType flag = MSG_INPUTS;
    oss.write(reinterpret_cast<const char*>(&flag), sizeof(PackageType));
    oss.write(reinterpret_cast<const char*>(&_id), sizeof(uint8_t));
    const uint16_t fps = fps_handler.get();
    oss.write(reinterpret_cast<const char*>(&fps), sizeof(uint16_t));
    uint8_t ping = _pingHandler.get();
    oss.write(reinterpret_cast<const char*>(&ping), sizeof(uint8_t));
    // Stocke le temps courant (en micro ou millisecondes)
    uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    oss.write(reinterpret_cast<const char*>(&now), sizeof(uint64_t));
    oss.write(reinterpret_cast<const char*>(_inputs), sizeof(uint8_t));

    // Vector2 position mouse
    if (hasFlag(*_inputs, Input::Shoot)) {
        oss.write(reinterpret_cast<const char*>(&_directionShoot.x()), sizeof(float));
        oss.write(reinterpret_cast<const char*>(&_directionShoot.y()), sizeof(float));
        oss.write(reinterpret_cast<const char*>(&_directionShoot.z()), sizeof(float));
        oss.write(reinterpret_cast<const char*>(&_translateShoot.x()), sizeof(float));
        oss.write(reinterpret_cast<const char*>(&_translateShoot.y()), sizeof(float));
        oss.write(reinterpret_cast<const char*>(&_translateShoot.z()), sizeof(float));
    }
    *_inputs = Input::None;

    std::string data = oss.str();
    ENetPacket* packet = enet_packet_create(
        data.data(), data.size(),
        ENET_PACKET_FLAG_RELIABLE
    );

    enet_peer_send(_peer, 0, packet);
}


void Client::pointerPressEvent(PointerEvent &event) {
    if(_imgui.handlePointerPressEvent(event)) return;
    *_inputs = *_inputs | Input::Shoot;
    const Vector2 position = event.position()*Vector2{framebufferSize()}/Vector2{windowSize()};
    const Vector2 clickPoint = Vector2::yScale(-1.0f)*(position/Vector2{framebufferSize()} - Vector2{0.5f})*_camera->projectionSize();
    _directionShoot = btVector3((_cameraObject->absoluteTransformation().rotationScaling() * Vector3{clickPoint, -1.0f}).normalized());
    _translateShoot = _cameraObject->absoluteTransformation().translation();
}

void Client::pointerReleaseEvent(PointerEvent &event) {
    if (_imgui.handlePointerReleaseEvent(event)) return;
    *_inputs = *_inputs & ~Input::Shoot;
}


void Client::keyPressEvent(KeyEvent &event) {
    if (_imgui.handleKeyPressEvent(event)) return;
    switch(event.key()) {
        case Key::W: *_inputs = *_inputs | Input::MoveForward; break;
        case Key::S: *_inputs = *_inputs | Input::MoveBackward; break;
        case Key::A: *_inputs = *_inputs | Input::MoveLeft; break;
        case Key::D: *_inputs = *_inputs | Input::MoveRight; break;
        case Key::Q: *_inputs = *_inputs | Input::MoveUp; break;
        case Key::E: *_inputs = *_inputs | Input::MoveDown; break;
        default: break;
    }
}

void Client::keyReleaseEvent(KeyEvent& event) {
    if (_imgui.handleKeyReleaseEvent(event)) return;
    switch (event.key()) {
        case Key::W: *_inputs = *_inputs & ~Input::MoveForward; break;
        case Key::S: *_inputs = *_inputs & ~Input::MoveBackward; break;
        case Key::A: *_inputs = *_inputs & ~Input::MoveLeft; break;
        case Key::D: *_inputs = *_inputs & ~Input::MoveRight; break;
        case Key::Q: *_inputs = *_inputs & ~Input::MoveUp; break;
        case Key::E: *_inputs = *_inputs & ~Input::MoveDown; break;
        default: break;
    }
}

void Client::textInputEvent(TextInputEvent& event) {
    std::cout << "Text input event: " << std::endl;
    if (_imgui.handleTextInputEvent(event)) return;
}

void Client::serialize(std::ostream&) const {}

void Client::unserialize(std::istream &istr) {
    // Unserialize frame number
    istr.read(reinterpret_cast<char*>(&_frame), sizeof(uint64_t));

    // Players
    uint8_t number_of_players;
    istr.read(reinterpret_cast<char*>(&number_of_players), sizeof(uint8_t));
    for (int i = 0; i < number_of_players; i++) {
        if (_players[i] == nullptr) {
            _players[i] = new Player(nullptr, this, &_scene);
        }
        _players[i]->unserialize(istr);
        _players[i]->updateBulletFromData();

        if (i == _id) {
            Vector3 target = {0.0f, 3.0f, 0.0f};
            Vector3 up = Vector3::yAxis();
            Matrix4 viewMatrix = Matrix4::lookAt(Vector3(_players[i]->_location), target, up);
            _cameraObject->setTransformation(viewMatrix);
        }
    }

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
