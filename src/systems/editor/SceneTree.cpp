//
// Created by Hugo Girard & Albin Horlaville on 17/03/2025.
//

#include "systems/editor/SceneTree.h"
#include "entities/GameObject.h"
#include "imgui.h"

SceneTree::SceneTree(std::map<std::string, GameObject*>* gameObjects) {
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
    for (auto const& pair : *_gameObjects) {
        sceneChildrenNodes.push_back(SceneNode(pair.second->_name, std::vector<SceneNode>{}));
    }
    auto root = SceneNode("Scene", sceneChildrenNodes);
    DrawNode(root);

    ImGui::End();

}