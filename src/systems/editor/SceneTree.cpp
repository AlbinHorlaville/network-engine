//
// Created by Hugo Girard & Albin Horlaville on 17/03/2025.
//

#include "systems/editor/SceneTree.h"
#include "entities/GameObject.h"
#include "imgui.h"

SceneTree::SceneTree(std::list<GameObject*>* gameObjects) {
    _gameObjects = gameObjects;
}

void SceneTree::DrawNode(SceneNode& node) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
    if (node.children.size() > 0)
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }
    if (ImGui::TreeNodeEx(node.name.c_str(), flags)) { // Expandable node
        for (auto& child : node.children) {
            DrawNode(child); // Recursive call for children
        }
        ImGui::TreePop(); // Close node
    }
}

void SceneTree::DrawSceneTree() {
    ImGui::Begin("Scene Hierarchy");
    std::vector<SceneNode> sceneChildrenNodes;
    for (const auto& object : *_gameObjects) {
        sceneChildrenNodes.push_back(SceneNode(object->_name, std::vector<SceneNode>{}));
    }
    auto root = SceneNode("Scene", sceneChildrenNodes);
    DrawNode(root);

    ImGui::End();

}