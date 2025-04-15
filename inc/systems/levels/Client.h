//
// Created by User on 08/04/2025.
//

#ifndef CLIENT_H
#define CLIENT_H

#include "enet6/enet.h"
#include "entities/primitives/Player.h"
#include "Engine.h"

class Client : public Engine {
    public:
        explicit Client(const Arguments &arguments);
        ~Client();

    private:
        uint8_t _id; // Send by the server, from 0 to 3.
        ENetHost* _client;
        ENetPeer* _peer;
        Player* _players[4];
        uint8_t _frame = 0;

    public:
        void tickEvent() override;
        void networkUpdate() override;
        void handleReceive(const ENetEvent &event);
        void handleDisconnect(const ENetEvent &event);
        void sendSnapshotACK(ENetPeer *peer);
        void initENet6();
        void pointerPressEvent(PointerEvent& event) override;
        void keyPressEvent(KeyEvent& event) override;
        void serialize(std::ostream &ostr) const override;
        void unserialize(std::istream &istr) override;
};



#endif //CLIENT_H
