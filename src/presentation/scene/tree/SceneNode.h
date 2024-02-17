//
// Created by faliszewskii on 07.01.24.
//

#ifndef OPENGL_SANDBOX_SCENENODE_H
#define OPENGL_SANDBOX_SCENENODE_H


#define UUID_SYSTEM_GENERATOR
#include "../../../../lib/uuid/uuid.h"
#include "../visitor/SceneNodeVisitor.h"
#include <string>
#include <stack>
#include <ranges>
#include <iostream>

class SceneNode {
    uuids::uuid uniqueObjectId;
    std::string name;
    std::vector<std::reference_wrapper<SceneNode>> children; // SceneNode should be notified if any child is deleted. Undefined behavior otherwise.
public:
    explicit SceneNode(std::string name) : name(std::move(name)) {
        uniqueObjectId = uuids::uuid_system_generator{}();
    }

    [[nodiscard]] uuids::uuid getUuid() const { return uniqueObjectId; };
    [[nodiscard]] std::string getName() const { return name; };
    virtual std::string getTypeName() = 0;

    void addChild(SceneNode &childValue) { children.emplace_back(childValue); }

    virtual int acceptVisit(SceneNodeVisitor &visitor) = 0;
    virtual int acceptLeave(SceneNodeVisitor &visitor) { return 0; };

    static void visitTree(SceneNode &sceneNode, SceneNodeVisitor &visitor) {
        std::stack<std::pair<std::reference_wrapper<SceneNode>, int>> stack{};
        stack.emplace(sceneNode, 0);
        while (!stack.empty()) {
            SceneNode &node = stack.top().first;
            int state = stack.top().second;
            stack.pop();
            if (state == 0) {
                stack.emplace(node, 1);
                if (!node.acceptVisit(visitor))
                    for (auto &child: std::ranges::views::reverse(node.children))
                        stack.emplace(child, 0);
            }
            if (state == 1) node.acceptLeave(visitor);
        }
    }

    friend bool operator==(SceneNode &lhs, SceneNode &rhs) {
        return std::addressof(lhs) == std::addressof(rhs);
    }

    virtual ~SceneNode() = default;
    SceneNode (const SceneNode&) = delete;
    SceneNode& operator= (const SceneNode&) = delete;
    SceneNode(SceneNode &&other) noexcept = default;
    SceneNode& operator=(SceneNode &&other) = default;

protected:
    SceneNode() = default;
};


#endif //OPENGL_SANDBOX_SCENENODE_H
