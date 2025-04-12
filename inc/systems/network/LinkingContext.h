//
// Created by Albin Horlaville on 31/03/2025.
//

#ifndef LINKINGCONTEXT_H
#define LINKINGCONTEXT_H

#include <stdint.h>
#include <unordered_map>

class GameObject;

class LinkingContext {
public:
    uint32_t GetNetworkID(GameObject* object);
    GameObject* GetLocalObject(uint32_t networkID);

    void Register(GameObject* object);
    void Register(uint32_t key, GameObject* object);
    void Unregister(GameObject* object);

private:
    std::unordered_map<GameObject*, uint32_t> localToNetwork;
    std::unordered_map<uint32_t, GameObject*> networkToLocal;
    uint32_t _lastID = 0;
};

#endif //LINKINGCONTEXT_H
