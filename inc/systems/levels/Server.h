//
// Created by Albin Horlaville on 08/04/2025.
//

#ifndef SERVER_H
#define SERVER_H

#include "Engine.h"
#include "enet6/enet.h"
#include "systems/network/PlayerData.h"

class Server : public Engine {
    public:
        explicit Server(const Arguments &arguments);
        ~Server();
    private:
        ENetHost* _server;
        PlayerData* _players[4];

        void tickEvent() override;
        void networkUpdate() override;
        void initENet6();
    public:
        void pointerPressEvent(PointerEvent& event) override;
        void keyPressEvent(KeyEvent& event) override;
};



#endif //SERVER_H
