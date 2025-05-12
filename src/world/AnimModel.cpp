#include "AnimModel.hpp"

#include "../render/BufferManager.hpp"
#include "../render/Constants.hpp"

using namespace world;
using namespace render;

AnimModelBlueprint::AnimModelBlueprint(const std::string& path,
                                       VkFormat format) {
    texture = BufferManager::get().allocateTexture(path, format);

    Joint joint{JointId::root(), GeometryMesh{}};

    Transform transform{
        glm::vec3{0.0f, 0.0f, 0.0f},
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3{0.0f, 0.0f, 0.0f},
        glm::vec3(0.0f, 0.0f, 0.0f),
    };

    joints.push_back(std::move(joint));
    transforms.push_back(transform);
}

AnimModelBlueprint::JointId AnimModelBlueprint::addJoint(
    JointId parent, glm::ivec3 center, glm::ivec3 pos, glm::quat rot,
    glm::ivec3 size, glm::ivec2 topLeft) {
    size_t index = joints.size();

    auto topBounds = getBounds(topLeft, Side::TOP, size);
    auto bottomBounds = getBounds(topLeft, Side::BOTTOM, size);
    auto zNegBounds = getBounds(topLeft, Side::SIDE_Z_NEG, size);
    auto zPosBounds = getBounds(topLeft, Side::SIDE_Z_POS, size);
    auto xNegBounds = getBounds(topLeft, Side::SIDE_X_NEG, size);
    auto xPosBounds = getBounds(topLeft, Side::SIDE_X_POS, size);

    glm::vec3 xPosYNegZPos = {
        convertWorldIntCoord(size.x - center.x),
        convertWorldIntCoord(-center.y),
        convertWorldIntCoord(size.z - center.z),
    };

    glm::vec3 xPosYNegZNeg = {
        convertWorldIntCoord(size.x - center.x),
        convertWorldIntCoord(-center.y),
        convertWorldIntCoord(-center.z),
    };

    glm::vec3 xNegYNegZPos = {
        convertWorldIntCoord(-center.x),
        convertWorldIntCoord(-center.y),
        convertWorldIntCoord(size.z - center.z),
    };

    glm::vec3 xNegYNegZNeg = {
        convertWorldIntCoord(-center.x),
        convertWorldIntCoord(-center.y),
        convertWorldIntCoord(-center.z),
    };

    glm::vec3 xPosYPosZPos = {
        convertWorldIntCoord(size.x - center.x),
        convertWorldIntCoord(size.y - center.y),
        convertWorldIntCoord(size.z - center.z),
    };

    glm::vec3 xPosYPosZNeg = {
        convertWorldIntCoord(size.x - center.x),
        convertWorldIntCoord(size.y - center.y),
        convertWorldIntCoord(-center.z),
    };

    glm::vec3 xNegYPosZPos = {
        convertWorldIntCoord(-center.x),
        convertWorldIntCoord(size.y - center.y),
        convertWorldIntCoord(size.z - center.z),
    };

    glm::vec3 xNegYPosZNeg = {
        convertWorldIntCoord(-center.x),
        convertWorldIntCoord(size.y - center.y),
        convertWorldIntCoord(-center.z),
    };

    // clang-format off
    GeometryMesh mesh = BufferManager::get().allocateMesh<GeometryMesh>({
            0,  1,  2,  0,  2,  3,
            4,  5,  6,  4,  6,  7,
    
            8,  9,  10, 8,  10, 11,
            12, 13, 14, 12, 14, 15,
    
            16, 17, 18, 16, 18, 19,
            20, 21, 22, 20, 22, 23,
        }, {
            // Z- side
            {xPosYNegZNeg, Z_NEG_NORMAL, Z_NEG_TANGENT, zNegBounds.getBottomRight()},
            {xNegYNegZNeg, Z_NEG_NORMAL, Z_NEG_TANGENT, zNegBounds.getBottomLeft()},
            {xNegYPosZNeg, Z_NEG_NORMAL, Z_NEG_TANGENT, zNegBounds.getTopLeft()},
            {xPosYPosZNeg, Z_NEG_NORMAL, Z_NEG_TANGENT, zNegBounds.getTopRight()},

            // Z+ side
            {xNegYNegZPos, Z_POS_NORMAL, Z_POS_TANGENT, zPosBounds.getBottomLeft()},
            {xPosYNegZPos, Z_POS_NORMAL, Z_POS_TANGENT, zPosBounds.getBottomRight()},
            {xPosYPosZPos, Z_POS_NORMAL, Z_POS_TANGENT, zPosBounds.getTopRight()},
            {xNegYPosZPos, Z_POS_NORMAL, Z_POS_TANGENT, zPosBounds.getTopLeft()},

            // Bottom
            {xNegYNegZNeg, Y_NEG_NORMAL, Y_NEG_TANGENT, bottomBounds.getBottomLeft()},
            {xPosYNegZNeg, Y_NEG_NORMAL, Y_NEG_TANGENT, bottomBounds.getBottomRight()},
            {xPosYNegZPos, Y_NEG_NORMAL, Y_NEG_TANGENT, bottomBounds.getTopRight()},
            {xNegYNegZPos, Y_NEG_NORMAL, Y_NEG_TANGENT, bottomBounds.getTopLeft()},

            // Top
            {xNegYPosZPos, Y_POS_NORMAL, Y_POS_TANGENT, topBounds.getBottomLeft()},
            {xPosYPosZPos, Y_POS_NORMAL, Y_POS_TANGENT, topBounds.getBottomRight()},
            {xPosYPosZNeg, Y_POS_NORMAL, Y_POS_TANGENT, topBounds.getTopRight()},
            {xNegYPosZNeg, Y_POS_NORMAL, Y_POS_TANGENT, topBounds.getTopLeft()},

            // X- side
            {xNegYNegZNeg, X_NEG_NORMAL, X_NEG_TANGENT, xNegBounds.getBottomRight()},
            {xNegYNegZPos, X_NEG_NORMAL, X_NEG_TANGENT, xNegBounds.getBottomLeft()},
            {xNegYPosZPos, X_NEG_NORMAL, X_NEG_TANGENT, xNegBounds.getTopLeft()},
            {xNegYPosZNeg, X_NEG_NORMAL, X_NEG_TANGENT, xNegBounds.getTopRight()},

            // X+ side
            {xPosYNegZPos, X_POS_NORMAL, X_POS_TANGENT, xPosBounds.getBottomLeft()},
            {xPosYNegZNeg, X_POS_NORMAL, X_POS_TANGENT, xPosBounds.getBottomRight()},
            {xPosYPosZNeg, X_POS_NORMAL, X_POS_TANGENT, xPosBounds.getTopRight()},
            {xPosYPosZPos, X_POS_NORMAL, X_POS_TANGENT, xPosBounds.getTopLeft()}
        });
    // clang-format on

    Joint joint{parent, std::move(mesh)};

    Transform transform{
        {
            convertWorldIntCoord(pos.x),
            convertWorldIntCoord(pos.y),
            convertWorldIntCoord(pos.z),
        },
        rot,
        glm::vec3{0.0f, 0.0f, 0.0f},
        glm::vec3(0.0f, 0.0f, 0.0f),
    };

    joints.push_back(std::move(joint));
    transforms.push_back(transform);

    return {index};
}

AnimModelPose AnimModelBlueprint::newPose() const { return {transforms}; }

void AnimModelBlueprint::addToModelList(
    std::list<render::GeometryModel>& models, AnimModelPose& pose) {
    pose.transforms[0].updateGlobal();

    for (int i = 1; i < joints.size(); i++) {
        int parent_idx = joints[i].parent.index;
        pose.transforms[i].updateGlobal(pose.transforms[parent_idx]);
    }

    for (int i = 0; i < joints.size(); i++) {
        // printf("Yeet: %d %p %p\n", i, *joints[i].mesh.buffer,
        // *texture.image.image);
        GeometryModel model{joints[i].mesh, texture,
                            pose.transforms[i].globalPos,
                            pose.transforms[i].globalRot};

        models.push_back(model);
    }
}

AnimModelBlueprint::Bounds AnimModelBlueprint::getBounds(
    glm::ivec2 topLeft, Side side, glm::ivec3 size) const {
    glm::ivec2 sideSize;
    if (side == Side::TOP || side == Side::BOTTOM) {
        sideSize = glm::ivec2{size.x, size.z};
    } else if (side == Side::SIDE_X_POS || side == Side::SIDE_X_NEG) {
        sideSize = glm::ivec2{size.z, size.y};
    } else if (side == Side::SIDE_Z_POS || side == Side::SIDE_Z_NEG) {
        sideSize = glm::ivec2{size.x, size.y};
    }

    glm::ivec2 sideTopLeft = topLeft;
    if (side == Side::TOP) {
        // sideTopLeft = topLeft;
    } else if (side == Side::BOTTOM) {
        sideTopLeft += glm::ivec2{size.x, 0};
    } else if (side == Side::SIDE_Z_POS) {
        sideTopLeft += glm::ivec2{0, size.z};
    } else if (side == Side::SIDE_Z_NEG) {
        sideTopLeft += glm::ivec2{size.x, size.z};
    } else if (side == Side::SIDE_X_POS) {
        sideTopLeft += glm::ivec2{size.x * 2, size.z};
    } else if (side == Side::SIDE_X_NEG) {
        sideTopLeft += glm::ivec2{size.x * 2 + size.z, size.z};
    }

    return {convertTextureIntCoords(sideTopLeft),
            convertTextureIntCoords(sideTopLeft + sideSize)};
}

glm::vec2 AnimModelBlueprint::convertTextureIntCoords(glm::ivec2 coords) const {
    return {(float)coords.x / texture.image.width,
            (float)coords.y / texture.image.height};
}

float AnimModelBlueprint::convertWorldIntCoord(int coord) const {
    return (coord / 16.0f);
}