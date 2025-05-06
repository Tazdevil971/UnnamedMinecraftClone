#pragma once

#include <glm/glm.hpp>
#include <list>
#include <memory>
#include <vector>

#include "../render/Primitives.hpp"
#include "Block.hpp"

namespace world {

class AnimModelPose;

class AnimModelBlueprint {
    friend class AnimModelPose;

public:
    struct JointId {
        static constexpr JointId root() { return {0}; }

        size_t index;
    };

private:
    struct Joint {
        JointId parent;
        render::GeometryMesh mesh;

        Joint() = default;
        Joint(Joint&&) = default;
        Joint& operator=(Joint&&) = default;
    };

    struct Transform {
        glm::vec3 localPos;
        glm::quat localRot;
        glm::vec3 globalPos;
        glm::quat globalRot;

        void updateGlobal() {
            globalPos = localPos;
            globalRot = localRot;
        }

        void updateGlobal(const Transform& parent) {
            globalPos = parent.globalPos + parent.globalRot * localPos;
            globalRot = parent.globalRot * localRot;
        }
    };

public:
    AnimModelBlueprint(const std::string& path, VkFormat format);

    JointId addJoint(JointId parent, glm::ivec3 center, glm::ivec3 pos,
                     glm::quat rot, glm::ivec3 size, glm::ivec2 topLeft);

    AnimModelPose newPose() const;

    void addToModelList(std::list<render::GeometryModel>& models,
                        AnimModelPose& pose);

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
    std::vector<Transform> transforms;
};

class AnimModelPose {
    friend class AnimModelBlueprint;

private:
    AnimModelPose(const std::vector<AnimModelBlueprint::Transform>& transforms)
        : transforms{transforms} {}

public:
    void setPos(glm::vec3 pos) { transforms[0].localPos = pos; }
    void setRot(glm::quat rot) { transforms[0].localRot = rot; }
    void setJointRot(AnimModelBlueprint::JointId id, glm::quat rot) {
        transforms[id.index].localRot = rot;
    }

private:
    std::vector<AnimModelBlueprint::Transform> transforms;
};

}  // namespace world