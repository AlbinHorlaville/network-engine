//
// Created by User on 14/03/2025.
//

#ifndef PHYSICSWORLD_H
#define PHYSICSWORLD_H

#include <btBulletDynamicsCommon.h>
#include <memory>

class PhysicsWorld {
private:
    std::unique_ptr<btBroadphaseInterface> _bBroadphase;
    std::unique_ptr<btDefaultCollisionConfiguration> _bCollisionConfig;
    std::unique_ptr<btCollisionDispatcher> _bDispatcher;
    std::unique_ptr<btSequentialImpulseConstraintSolver> _bSolver;

public:
    std::unique_ptr<btDiscreteDynamicsWorld> _bWorld;

    PhysicsWorld() = delete;
    explicit PhysicsWorld(btVector3 gravity);
    ~PhysicsWorld() = default;

    void setGravity(btVector3 gravity) const;

    // Remove objects that are too far from the origin
    void cleanWorld() const;
};



#endif //PHYSICSWORLD_H
