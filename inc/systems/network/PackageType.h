//
// Created by User on 09/04/2025.
//

#ifndef PACKAGETYPE_H
#define PACKAGETYPE_H

enum PackageType : uint8_t {
    MSG_ASSIGN_ID = 0,
    MSG_WORLD_SYNC = 1,
    MSG_WORLD_ACK = 2
};

#endif //PACKAGETYPE_H
