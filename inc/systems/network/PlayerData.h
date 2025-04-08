//
// Created by User on 08/04/2025.
//

#ifndef PLAYERDATA_H
#define PLAYERDATA_H

#include <cstdint>

struct PlayerData {
    uint8_t id; // 0, 1, 2 or 3
    const char *ip;
    uint16_t port;

    // Peut être autre chose ????
};

#endif //PLAYERDATA_H
