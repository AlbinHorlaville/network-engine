//
// Created by Hugo Girard & Albin Horlaville on 17/03/2025.
//

#ifndef SCENETREE_H
#define SCENETREE_H

#include <list>
#include <map>
#include <string>
#include <vector>

class GameObject;

struct SceneNode {
    std::string name;
    std::vector<SceneNode> children;

    SceneNode(std::string n, std::vector<SceneNode> c) : name(n), children(c) {}
};

class SceneTree {

public:
    SceneTree() = delete;
    SceneTree(std::map<std::string, GameObject*>* gameObjects);
    ~SceneTree() = default;

    // Draw the entire scene tree
    void DrawSceneTree();

private:
    // Draw a given node of the scene tree
    void DrawNode(SceneNode& node);

    std::map<std::string, GameObject *> *_gameObjects;

};

#endif //SCENETREE_H
