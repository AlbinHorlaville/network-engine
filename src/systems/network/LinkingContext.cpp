//
// Created by Albin Horlaville on 31/03/2025.
//

#include "systems/network/LinkingContext.h"
#include "entities/GameObject.h"

uint32_t LinkingContext::GetNetworkID(GameObject* object) {
    auto it = localToNetwork.find(object);
    return (it != localToNetwork.end()) ? it->second : 0; // 0 means not found
}

GameObject* LinkingContext::GetLocalObject(uint32_t networkID) {
    auto it = networkToLocal.find(networkID);
    return (it != networkToLocal.end()) ? it->second : nullptr;
}

void LinkingContext::Register(GameObject* object) {
    localToNetwork[object] = _lastID;
    networkToLocal[_lastID] = object;
    object->_id = _lastID;
    _lastID++;
}

void LinkingContext::Register(uint32_t key, GameObject* object) {
    localToNetwork[object] = key;
    networkToLocal[key] = object;
    object->_id = key;
}

void LinkingContext::Unregister(GameObject* object) {
    auto it = localToNetwork.find(object);
    if (it != localToNetwork.end()) {
        networkToLocal.erase(it->second);
        localToNetwork.erase(it);
    }
}
