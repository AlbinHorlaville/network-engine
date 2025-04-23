//
// Created by Albin Horlaville on 23/04/2025.
//

#ifndef INTERPOLATION_H
#define INTERPOLATION_H

#include <deque>

struct EntityState {
    uint64_t timestamp = 0;
    btVector3 position;
    btQuaternion rotation;
};

struct EntityInterpolation {
    std::deque<EntityState> history;
};

#endif //INTERPOLATION_H
