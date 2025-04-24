#pragma once

#include <glm/glm.hpp>
#include <list>
#include <string>
#include <vector>

#include "../render/Primitives.hpp"
#include "Block.hpp"

namespace world {

class AnimModel {
public:
    struct JointId {
        static constexpr JointId root() { return {0}; }

        int index;
    };

private:
    struct Joint {
        JointId parent;
        glm::vec3 localPos;
        glm::quat localRot;
        render::GeometryMesh mesh;
        glm::vec3 globalPos;
        glm::quat globalRot;

        void updateGlobal() {
            globalPos = localPos;
            globalRot = localRot;
        }

        void updateGlobal(const Joint& parent) {
            globalPos = parent.globalPos + parent.globalRot * localPos;
            globalRot = parent.globalRot * localRot;
        }
    };

public:
    AnimModel(const std::string& path, VkFormat format);

    void cleanup();

    JointId addJoint(JointId parent, glm::ivec3 center, glm::ivec3 pos,
                     glm::quat rot, glm::ivec3 size, glm::ivec2 topLeft);

    void setPos(glm::vec3 pos) { joints[0].localPos = pos; }
    void setRot(glm::quat rot) { joints[0].localRot = rot; }
    void setJointRot(JointId id, glm::quat rot) {
        joints[id.index].localRot = rot;
    }

    void addToModelList(std::list<render::GeometryModel>& models);

private:
    struct Bounds {
        glm::vec2 topLeft;
        glm::vec2 bottomRight;

        glm::vec2 getTopLeft() const { return topLeft; }
        glm::vec2 getBottomRight() const { return bottomRight; }
        glm::vec2 getTopRight() const { return {bottomRight.x, topLeft.y}; }
        glm::vec2 getBottomLeft() const { return {topLeft.x, bottomRight.y}; }
    };

    Bounds getBounds(glm::ivec2 topLeft, Side side, glm::ivec3 size) const;
    glm::vec2 convertTextureIntCoords(glm::ivec2 coords) const;
    float convertWorldIntCoord(int coord) const;

    render::Texture texture;
    std::vector<Joint> joints;
};

}  // namespace world