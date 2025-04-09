//
// Created by User on 08/04/2025.
//

#ifndef CLIENT_H
#define CLIENT_H

#include "enet6/enet.h"
#include "Engine.h"

class Client : public Engine {
    public:
        explicit Client(const Arguments &arguments);
        ~Client();

    private:
        uint8_t _id; // Send by the server, from 0 to 3.
        ENetHost* _client;
        ENetPeer* _peer;


    void tickEvent() override;
    void networkUpdate() override;
    void initENet6();
    void pointerPressEvent(PointerEvent& event) override;
    void keyPressEvent(KeyEvent& event) override;
};



#endif //CLIENT_H
