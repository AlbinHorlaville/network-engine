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
    _httpClient = HttpClient();
    setSwapInterval(1); // optional vsync
    redraw();
    _cameraObject = new Object3D{&_scene};
    (_camera = new SceneGraph::Camera3D(*_cameraObject))
    ->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
    .setProjectionMatrix(Matrix4::perspectiveProjection(35.0_degf, 1.0f, 0.001f, 99.0f))
    .setViewport(GL::defaultFramebuffer.viewport().size());
}

Client::~Client() {
    _httpClient.unqueuePlayer();
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
    std::string raw = _currentServerIp;
    std::string ip;
    uint16_t port = 0;

    if (raw.front() == '[') {
        // IPv6 wrapped in square brackets
        auto endBracket = raw.find(']');
        if (endBracket != std::string::npos && endBracket + 1 < raw.size() && raw[endBracket + 1] == ':') {
            ip = raw.substr(1, endBracket - 1);  // e.g., "::"
            try {
                port = static_cast<uint16_t>(std::stoi(raw.substr(endBracket + 2)));
            } catch (const std::exception& e) {
                std::cerr << "Invalid port format in: " << raw << " - " << e.what() << std::endl;
                return;
            }
        } else {
            std::cerr << "Malformed IPv6 address: " << raw << std::endl;
            return;
        }
    } else {
        // IPv4 or domain name
        auto colon = raw.rfind(':');
        if (colon != std::string::npos && colon + 1 < raw.size()) {
            ip = raw.substr(0, colon);
            try {
                port = static_cast<uint16_t>(std::stoi(raw.substr(colon + 1)));
            } catch (const std::exception& e) {
                std::cerr << "Invalid port format in: " << raw << " - " << e.what() << std::endl;
                return;
            }
        } else {
            std::cerr << "Malformed address: " << raw << std::endl;
            return;
        }
    }

    if (ip == "::") {
        ip = "::1";
    }
    // Set address
    enet_address_set_host(&address, ENET_ADDRESS_TYPE_IPV6, ip.c_str());
    address.port = port;

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
    _pingHandler.init();
    _inputs = new Input(Input::None);
}

void Client::tickEvent() {
    glfwPollEvents();

    switch(_state) {
        case (InGame) :
            if (_client) {
                networkUpdate();
                interpolate();
                _currentTimeServer += _timeline.previousFrameDuration() * 1000;
                // Simulation physique
                _pWorld->_bWorld->stepSimulation(_timeline.previousFrameDuration(), 5);
                sendInputs();

                // Avance la timeline et redessine
                _timeline.nextFrame();
            }
        break;
        case (Queue) :
            _currentServerIp = _httpClient.getMatchStatus();
            if(!_currentServerIp.empty()) {
                initSimulation();
                // RX initialisation
                initENet6();
                _state = InGame;
            }
        break;
        case (EndGame) :
            // Rien à faire pour l'instant
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
        enet_packet_destroy(event.packet);
    }
}

void Client::handleReceive(const ENetEvent &event) {
    size_t payloadSize = event.packet->dataLength;
    const char* payload = reinterpret_cast<const char*>(event.packet->data);
    std::istringstream iss(std::string(payload, payloadSize), std::ios::binary);
    PackageType type;
    iss.read(reinterpret_cast<char*>(&type), sizeof(type));

    switch (type) {
        case MSG_ASSIGN_ID: {
            iss.read(reinterpret_cast<char*>(&_id), sizeof(uint8_t));
            sendUsername(event.peer);
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

            uint64_t now = getTime();
            _pingHandler.update(sentTime, now);
            break;
        }
        case MSG_END_GAME: {
            _state = EndGame;

            std::map<std::string, std::string> newAchievementsMap = _httpClient.getDetailedAchievements();
            _unlockedThisGame.clear();

            for (const auto& pair : newAchievementsMap) {
                if (!_achievements.contains(pair.first)) {
                    _unlockedThisGame[pair.first] = pair.second;
                }
            }
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
        case (InGame):
            drawGraphics();
            drawImGUI();
        break;
        case (WelcomeScreen) :
            _imgui.newFrame();
            drawLoginWindow();
            endFrame();
        break;
        case (Queue) :
            _imgui.newFrame();
            drawQueueWindow();
            endFrame();
        break;
        case (Lobby) :
            _imgui.newFrame();
            drawLobbyWindow();
            endFrame();
        break;
        case (Stats) :
            _imgui.newFrame();
            drawStatsWindow();
            endFrame();
        break;
        case(Achievements) :
            _imgui.newFrame();
            drawAchievementsWindow();
            endFrame();
        break;
        case (EndGame):
            _imgui.newFrame();
            drawEndGameWindow();
            endFrame();
            break;
        default:
            break;
    }

    swapBuffers();
    redraw();
}

void Client::drawQueueWindow() {
    ImVec2 windowSize = ImGui::GetMainViewport()->Size;
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always, ImVec2(0.0f, 0.0f)); // Place window in top-left corner
    ImGui::SetNextWindowSize(ImVec2(windowSize.x, windowSize.y)); // Set a dynamic size corresponding to parent window size

    ImGui::Begin("Queue", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Waiting for players and server...");

    if (ImGui::Button("Leave Queue")) {
        if(_httpClient.unqueuePlayer()) {
            _state = Lobby;
        } else {
            std::cerr << "Failed to leave queue." << std::endl;
        }
    }

    ImGui::End();
}

void Client::drawLobbyWindow() {
    ImVec2 windowSize = ImGui::GetMainViewport()->Size;
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always, ImVec2(0.0f, 0.0f)); // Place window in top-left corner
    ImGui::SetNextWindowSize(ImVec2(windowSize.x, windowSize.y)); // Set a dynamic size corresponding to parent window size

    ImGui::Begin("Lobby", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::Button("Queue")) {
        if(_httpClient.queuePlayer()) _state = Queue;
    }

    if (ImGui::Button("Stats")) {
        _stats = _httpClient.getStatsParsed();
        _state = Stats;
    }

    if (ImGui::Button("Achievements")) {
        _achievements = _httpClient.getDetailedAchievements();
        _state = Achievements;
    }

    if (ImGui::Button("Disconnect")) {
        _state = WelcomeScreen;
    }

    ImGui::End();
}

void Client::drawLoginWindow() {
    ImVec2 windowSize = ImGui::GetMainViewport()->Size;
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always, ImVec2(0.0f, 0.0f)); // Place window in top-left corner
    ImGui::SetNextWindowSize(ImVec2(windowSize.x, windowSize.y)); // Set a dynamic size corresponding to parent window size

    ImGui::Begin("Welcome", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    static char usernameLogin[64] = "";
    static char passwordLogin[64] = "";

    if (ImGui::RadioButton("Login", connectTypeOption == 0))
    {
        connectTypeOption = 0;
        _loginProblem = false;
    }

    if (ImGui::RadioButton("Register", connectTypeOption == 1))
    {
        connectTypeOption = 1;
        _loginProblem = false;
    }

    if (ImGui::InputText("Username", usernameLogin, IM_ARRAYSIZE(usernameLogin), ImGuiInputTextFlags_None)) {
        // You can add additional logic here for handling username input
    }
    if (ImGui::InputText("Password", passwordLogin, IM_ARRAYSIZE(passwordLogin), ImGuiInputTextFlags_Password)) {
        // You can add additional logic here for handling password input
    }

    if (ImGui::Button("Connect")) {
        if (connectTypeOption == 0) //Login mode
        {
            if (_httpClient.login(usernameLogin, passwordLogin)) {
                _state = Lobby;
                _username = usernameLogin;
                std::memset(usernameLogin, 0, sizeof(usernameLogin));
                std::memset(passwordLogin, 0, sizeof(passwordLogin));
                _loginProblem = false;
            } else {
                _loginProblem = true;
            }
        }
        else //Register mode
        {
            if (_httpClient.registerUser(usernameLogin, passwordLogin)) {
                _state = Lobby;
                _username = usernameLogin;
                std::memset(usernameLogin, 0, sizeof(usernameLogin));
                std::memset(passwordLogin, 0, sizeof(passwordLogin));
                _loginProblem = false;
            } else {
                _loginProblem = true;
            }
        }
    }

    if (_loginProblem) {
        if (connectTypeOption == 0) {
            ImGui::Text("Username or password is incorrect.");
        } else {
            ImGui::Text("Username is already taken.");
        }
    }
    ImGui::End();
}

void Client::drawEndGameWindow() {
    ImVec2 windowSize = ImGui::GetMainViewport()->Size;
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always, ImVec2(0.0f, 0.0f)); // Place window in top-left corner
    ImGui::SetNextWindowSize(ImVec2(windowSize.x, windowSize.y)); // Set a dynamic size corresponding to parent window size

    ImGui::Begin("Game ended", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    std::vector<Player*> sortedPlayers(_players.begin(), _players.end());

    std::sort(sortedPlayers.begin(), sortedPlayers.end(), [](Player* a, Player* b) {
        return a->_score > b->_score;
    });

    for (size_t i = 0; i < sortedPlayers.size(); ++i) {
        ImGui::Text("%d - %s with %d cubes pushed.", i+1, sortedPlayers[i]->_name.c_str(), sortedPlayers[i]->_score);
    }

    ImGui::Spacing();

    ImGui::Text("Achievement(s) unlocked this game:");

    int index = 1;
    for (auto it = _unlockedThisGame.begin(); it != _unlockedThisGame.end(); ++it, ++index) {
        ImGui::Text("Achievement %d", index);
        ImGui::Text("%s", it->first.c_str());
        ImGui::Text("%s", it->second.c_str());
    }

    if (ImGui::Button("Lobby")) {
        _state = Lobby;
        reset();
    }

    ImGui::End();
}

void Client::drawStatsWindow() {
    ImVec2 windowSize = ImGui::GetMainViewport()->Size;
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always, ImVec2(0.0f, 0.0f)); // Place window in top-left corner
    ImGui::SetNextWindowSize(ImVec2(windowSize.x, windowSize.y)); // Set a dynamic size corresponding to parent window size

    ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("%s", _username.c_str());

    ImGui::Text("Games won: %d", _stats.gamesWon);
    ImGui::Text("Games played: %d", _stats.gamesPlayed);
    ImGui::Text("Cubes pushed: %d", _stats.cubesPushed);
    ImGui::Text("Max cubes in one game: %d", _stats.maxCubesPushedInOneGame);

    if (ImGui::Button("Lobby")) {
        _state = Lobby;
    }

    ImGui::End();
}

void Client::drawAchievementsWindow() {
    ImVec2 windowSize = ImGui::GetMainViewport()->Size;
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always, ImVec2(0.0f, 0.0f)); // Place window in top-left corner
    ImGui::SetNextWindowSize(ImVec2(windowSize.x, windowSize.y)); // Set a dynamic size corresponding to parent window size

    ImGui::Begin("Achievements", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("%s", _username.c_str());

    if (!_achievements.empty()) {
        int index = 1;
        for (auto it = _achievements.begin(); it != _achievements.end(); ++it, ++index) {
            ImGui::Text("Achievement %d", index);
            ImGui::Text("%s", it->first.c_str());
            ImGui::Text("%s", it->second.c_str());
        }
        ImGui::Spacing();
    } else {
        ImGui::Text("No achievement unlocked yet...");
    }

    if (ImGui::Button("Lobby")) {
        _state = Lobby;
    }

    ImGui::End();
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
    uint64_t now = getTime();
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
    if (_imgui.handleTextInputEvent(event)) return;
}

void Client::serialize(std::ostream&) const {}

void Client::unserialize(std::istream &istr) {
    // Get timeServer
    uint64_t serverTime;
    istr.read(reinterpret_cast<char*>(&serverTime), sizeof(uint64_t));
    _currentTimeServer = serverTime + _pingHandler.get() / 2; // Le temps actuel du serveur c'est le temps auquel il a émit le paquet plus le temps de trajet

    // Unserialize frame number
    istr.read(reinterpret_cast<char*>(&_frame), sizeof(uint64_t));

    // Players
    uint8_t number_of_players;
    istr.read(reinterpret_cast<char*>(&number_of_players), sizeof(uint8_t));
    for (int i = 0; i < number_of_players; i++) {
        if (_players[i] == nullptr) {
            _players[i] = new Player(nullptr, this, &_scene);
        }
        _players[i]->_serverTime = serverTime;
        _players[i]->unserialize(istr);
        //_players[i]->updateBulletFromData();

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
            obj->_serverTime = serverTime;
            obj->unserialize(istr);
            //obj->updateBulletFromData();
        }
        else { // L'objet doit être créé
            ObjectType type;
            istr.read(reinterpret_cast<char*>(&type), sizeof(ObjectType));
            switch(type) {
                case CUBE: {
                    auto cube = new Cube(this, &_scene);
                    cube->_serverTime = serverTime;
                    cube->unserialize(istr);
                    //cube->updateBulletFromData();
                    addObject(cube, id);
                    break;
                }
                case SPHERE: {
                    auto* sphere = new Sphere(this, &_scene);
                    sphere->_serverTime = serverTime;
                    sphere->unserialize(istr);
                    //sphere->updateBulletFromData();
                    addObject(sphere, id);
                    break;
                }
            }
        }
    }
}

void Client::endFrame() {
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
}

void Client::sendUsername(ENetPeer *peer) {
    std::ostringstream oss(std::ios::binary);

    // Mettre le flag
    PackageType flag = MSG_USERNAME;
    oss.write(reinterpret_cast<const char*>(&flag), sizeof(PackageType));
    oss.write(reinterpret_cast<const char*>(&_id), sizeof(uint8_t));
    size_t length = _username.length();
    oss.write(reinterpret_cast<const char*>(&length), sizeof(size_t));
    oss.write(_username.data(), length);

    std::string data = oss.str();
    ENetPacket* packet = enet_packet_create(
        data.data(), data.size(),
        ENET_PACKET_FLAG_RELIABLE
    );

    enet_peer_send(peer, 1, packet);
}

uint64_t Client::getTime() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

void Client::interpolate() {
    uint64_t renderTimestamp = _currentTimeServer - interpolationDelay;

    for (auto& player : _players) {
        if (!player) {
            continue;
        }
        auto& history = player->_entityStates.history;

        // Trouver les deux états encadrant renderTimestamp
        if (history.size() < 2) continue;

        for (size_t i = 0; i < history.size() - 1; ++i) {
            const auto& stateA = history[i];
            const auto& stateB = history[i + 1];

            if (stateA.timestamp <= renderTimestamp && renderTimestamp <= stateB.timestamp) {
                const double delta = stateB.timestamp - stateA.timestamp;
                if (delta == 0) continue;
                const double t = double(renderTimestamp - stateA.timestamp) / delta;
                btVector3 interpPos = lerp(stateA.position, stateB.position, t);
                btQuaternion interpRot = slerp(stateA.rotation, stateB.rotation, t);

                // Appliquer à l'entité
                player->_location = interpPos;
                player->_rotation = interpRot;
                player->updateBulletFromData();
                break;
            }
        }
    }

    for (auto& pair : _objects) {
        auto& object = pair.second;
        auto& history = pair.second->_entityStates.history;

        // Trouver les deux états encadrant renderTimestamp
        if (history.size() < 2) continue;

        for (size_t i = 0; i < history.size() - 1; ++i) {
            const auto& stateA = history[i];
            const auto& stateB = history[i + 1];

            if (stateA.timestamp <= renderTimestamp && renderTimestamp <= stateB.timestamp) {
                const double delta = stateB.timestamp - stateA.timestamp;
                if (delta == 0) continue;
                const double t = double(renderTimestamp - stateA.timestamp) / delta;
                btVector3 interpPos = lerp(stateA.position, stateB.position, t);
                btQuaternion interpRot = slerp(stateA.rotation, stateB.rotation, t);

                // Appliquer à l'entité
                object->_location = interpPos;
                object->_rotation = interpRot;
                object->updateBulletFromData();
                break;
            }
        }
    }
}

void Client::reset() {
    Engine::reset();
    _id = 5;
    delete _peer;
    _frame = 0;
    delete _inputs;
}

