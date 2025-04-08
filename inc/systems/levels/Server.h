//
// Created by User on 08/04/2025.
//

#ifndef SERVER_H
#define SERVER_H

#include "Engine.h"
#include "systems/network/PlayerData.h"

class ENetHost;

class Server : public Engine {
    private:
        ENetHost* _server;
        PlayerData* _players[4];
};



#endif //SERVER_H
