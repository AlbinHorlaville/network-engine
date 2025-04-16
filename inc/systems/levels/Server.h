//
// Created by Albin Horlaville on 08/04/2025.
//

#ifndef SERVER_H
#define SERVER_H

#include "Engine.h"
#include "enet6/enet.h"
#include "entities/primitives/Player.h"
#include <array>

struct DestroyedObject {
        uint32_t id;
        uint64_t frame;
};

struct CollisionCallback : public btCollisionWorld::ContactResultCallback {
    std::function<void(btRigidBody*, btRigidBody*)> onCollision;

    btScalar addSingleResult(btManifoldPoint& cp,
                             const btCollisionObjectWrapper* colObj0Wrap, int, int,
                             const btCollisionObjectWrapper* colObj1Wrap, int, int) override {
        auto* rbA = const_cast<btRigidBody*>(btRigidBody::upcast(colObj0Wrap->getCollisionObject()));
        auto* rbB = const_cast<btRigidBody*>(btRigidBody::upcast(colObj1Wrap->getCollisionObject()));
        if (rbA && rbB && onCollision)
            onCollision(rbA, rbB);
        return 0; // Bullet ignore la valeur de retour dans ce cas
    }
};

class Server : public Engine {
    public:
        explicit Server(const Arguments &arguments);
        ~Server();
    private:
        ENetHost* _server;
        std::array<Player*, 4> _players = { nullptr };
        float snapshotTimer = 0.0f;
        uint64_t _frame = 0;
        std::list<DestroyedObject*> _destroyedObjects;

    public:
        void tickEvent() override;
        void cleanWorld();
        void networkUpdate() override;
        void handleConnect(const ENetEvent &event);
        void handleReceive(const ENetEvent &event);
        void handleDisconnect(const ENetEvent &event);
        void handleCollision();
        void sendSnapshot();
        void initENet6();
        void serialize(std::ostream &ostr) const override;
        void unserialize(std::istream &istr) override;
};



#endif //SERVER_H
