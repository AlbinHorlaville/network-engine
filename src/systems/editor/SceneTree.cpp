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
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow
    | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (node.children.size() == 0)
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
    bool open = true;
    ImVec2 windowSize = ImGui::GetMainViewport()->Size;
    ImGui::SetNextWindowPos(ImVec2(3, 3), ImGuiCond_Always, ImVec2(0.0f, 0.0f)); // Place window in top-left corner
    ImGui::SetNextWindowSize(ImVec2(windowSize.x/6.f, windowSize.y - 5.f)); // Set a dynamic size corresponding to parent window size

    ImGui::Begin("Scene Hierarchy", &open, ImGuiWindowFlags_NoResize);
    std::vector<SceneNode> sceneChildrenNodes;
    for (auto const& pair : *_gameObjects) {
        sceneChildrenNodes.push_back(SceneNode(pair.second->_name, std::vector<SceneNode>{}));
    }
    auto root = SceneNode("Scene", sceneChildrenNodes);
    DrawNode(root);

    ImGui::End();

}