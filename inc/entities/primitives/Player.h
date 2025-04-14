//
// Created by User on 09/04/2025.
//

#ifndef PLAYER_H
#define PLAYER_H

#include "entities/GameObject.h"
#include "enet6/enet.h"

class Player : public GameObject {
public:
    uint8_t _playerID = -1;
    ENetPeer *_peer;

    Player(uint8_t id, ENetPeer *peer, Engine* app, Object3D *parent);
    ~Player() = default;

    void updateBulletFromData() override;
    void setColor(const Color3 &color) override;
    void serialize(std::ostream &ostr) const override;
    void unserialize(std::istream &istr) override;
private:
    void setMass(float mass){}

private:
    btBoxShape _collisionShape;
    btVector3 _scale;
};



#endif //PLAYER_H
