//
// Created by User on 08/04/2025.
//

#ifndef CLIENT_H
#define CLIENT_H

#include "Engine.h"

class ENetPeer;

class Client : public Engine {
    private:
        uint8_t _id; // Send by the server, from 0 to 3.
        ENetPeer* _peer;
};



#endif //CLIENT_H
