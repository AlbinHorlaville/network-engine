//
// Created by User on 09/04/2025.
//

#ifndef PACKAGETYPE_H
#define PACKAGETYPE_H

enum PackageType : uint8_t {
    MSG_ASSIGN_ID = 0,
    MSG_WORLD_SYNC = 1,
    MSG_WORLD_ACK = 2,
    MSG_INPUTS = 3,
    MSG_INPUTS_ACK = 4,
    MSG_END_GAME = 5,
    MSG_USERNAME = 6
};

enum class Input : uint8_t {
    None  = 0,
    MoveForward  = 1 << 0, // W
    MoveBackward = 1 << 1, // S
    MoveLeft     = 1 << 2, // A
    MoveRight    = 1 << 3, // D
    MoveUp       = 1 << 4, // Q
    MoveDown     = 1 << 5, // E
    Shoot        = 1 << 6  // Left click
};

inline Input operator|(Input a, Input b) {
    return static_cast<Input>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline Input operator&(Input a, Input b) {
    return static_cast<Input>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

inline bool hasFlag(Input value, Input flag) {
    return static_cast<uint8_t>(value & flag) != 0;
}

inline Input operator~(Input a) {
    return static_cast<Input>(~static_cast<uint8_t>(a));
}

#endif //PACKAGETYPE_H
