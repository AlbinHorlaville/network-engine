//
// Created by User on 08/04/2025.
//

#ifndef CLIENT_H
#define CLIENT_H

#include "enet6/enet.h"
#include "entities/primitives/Player.h"
#include "Engine.h"
#include "systems/network/PackageType.h"
#include "systems/network/PingHandler.h"

enum ClientState {
    Not_logged_in,
    Logged_in,
    In_queue,
    Found_match,
    In_game,
};

class Client : public Engine {
    public:
        explicit Client(const Arguments &arguments);
        ~Client();

    private:
        uint8_t _id = 5; // Send by the server, from 0 to 3.
        ENetHost* _client;
        ENetPeer* _peer;
        ClientState _state = Not_logged_in;
        int connectTypeOption = 0;
        PingHandler _pingHandler;
        uint64_t _frame = 0;
        Input* _inputs = nullptr;
        btVector3 _directionShoot;
        Vector3 _translateShoot;

    public:
        void tickEvent() override;
        void networkUpdate() override;
        void handleReceive(const ENetEvent &event);
        void handleDisconnect(const ENetEvent &event);
        void sendSnapshotACK(ENetPeer *peer);
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
        void drawLoginWindow();
};



#endif //CLIENT_H
