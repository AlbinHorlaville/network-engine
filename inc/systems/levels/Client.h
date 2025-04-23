//
// Created by User on 08/04/2025.
//

#ifndef CLIENT_H
#define CLIENT_H

#include "enet6/enet.h"
#include "entities/primitives/Player.h"
#include "Engine.h"
#include "../../../cmake-build-debug/_deps/bullet-src/examples/ThirdPartyLibs/clsocket/src/Host.h"
#include "systems/network/PackageType.h"
#include "systems/network/PingHandler.h"
#include "systems/online/HttpClient.h"

enum ClientState {
    WelcomeScreen,
    Lobby,
    Queue,
    Stats,
    FoundMatch,
    InGame,
    EndGame
};

class Client : public Engine {
    public:
        explicit Client(const Arguments &arguments);
        ~Client();

    private:
        uint8_t _id = 5; // Send by the server, from 0 to 3.
        std::string _username;
        ENetHost* _client;
        ENetPeer* _peer;
        ClientState _state = WelcomeScreen;
        int connectTypeOption = 0;
        PingHandler _pingHandler;
        uint64_t _frame = 0;
        Input* _inputs = nullptr;
        btVector3 _directionShoot;
        Vector3 _translateShoot;
        HttpClient _httpClient;
        std::string _currentServerIp;
        bool _loginProblem = false;
        PlayerStats _stats;
        constexpr double interpolationDelay = 0.1; // 100 ms
        uint64_t _currentTimeServer;

    public:
        void tickEvent() override;
        void networkUpdate() override;
        void handleReceive(const ENetEvent &event);
        void handleDisconnect(const ENetEvent &event);
        void sendSnapshotACK(ENetPeer *peer);
        void sendUsername(ENetPeer *peer);
        void sendInputs();
        void initENet6();
        void pointerPressEvent(PointerEvent& event) override;
        void pointerReleaseEvent(PointerEvent& event) override;
        void keyPressEvent(KeyEvent& event) override;
        void keyReleaseEvent(KeyEvent& event) override;
        void textInputEvent(TextInputEvent& event) override;
        void drawEvent() override;
        void serialize(std::ostream &ostr) const override;
        void unserialize(std::istream &istr) override;
        void endFrame();
        void drawLoginWindow();
        void drawLobbyWindow();
        void drawQueueWindow();
        void drawEndGameWindow();
        void drawStatsWindow();
        uint64_t now();
        void interpolate();
        void reset() override;
};



#endif //CLIENT_H
