#pragma once

#include <glm/glm.hpp>

#include "../world/World.hpp"

namespace logic {

struct BlockRange {
    glm::ivec3 corner;
    glm::ivec3 size;

    bool isInside(glm::ivec3 pos) const {
        return (pos.x >= corner.x && pos.x < corner.x + size.x) &&
               (pos.y >= corner.y && pos.y < corner.y + size.y) &&
               (pos.z >= corner.z && pos.z < corner.z + size.z);
    }

    template <typename F>
    void visit(const F &f) const {
        for (int x = corner.x; x < corner.x + size.x; x++) {
            for (int y = corner.y; y < corner.y + size.y; y++) {
                for (int z = corner.z; z < corner.z + size.z; z++) {
                    if (!f(glm::ivec3{x, y, z})) {
                        return;
                    }
                }
            }
        }
    }
};

struct BoxCollider {
    glm::vec3 pos;
    glm::vec3 size;

    BlockRange getBlockRange() const {
        glm::ivec3 corner1 = glm::floor(
            glm::vec3{pos.x - size.x / 2, pos.y, pos.z - size.z / 2});
        glm::ivec3 corner2 = glm::ceil(
            glm::vec3{pos.x + size.x / 2, pos.y + size.y, pos.z + size.z / 2});

        return BlockRange{corner1, corner2 - corner1};
    }

    BlockRange getBottomBlockRange() const {
        glm::ivec3 corner1 =
            glm::ivec3{glm::floor(pos.x - size.x / 2), glm::floor(pos.y - 1),
                       glm::floor(pos.z - size.z / 2)};
        glm::ivec3 corner2 =
            glm::ivec3{glm::ceil(pos.x + size.x / 2), glm::floor(pos.y),
                       glm::ceil(pos.z + size.z / 2)};

        return BlockRange{corner1, corner2 - corner1};
    }

    BlockRange getTopBlockRange() const {
        glm::ivec3 corner1 = glm::ivec3{glm::floor(pos.x - size.x / 2),
                                        glm::ceil(pos.y + size.y),
                                        glm::floor(pos.z - size.z / 2)};
        glm::ivec3 corner2 = glm::ivec3{glm::ceil(pos.x + size.x / 2),
                                        glm::ceil(pos.y + size.y + 1),
                                        glm::ceil(pos.z + size.z / 2)};

        return BlockRange{corner1, corner2 - corner1};
    }

    BlockRange getZPosBlockRange() const {
        glm::ivec3 corner1 =
            glm::ivec3{glm::floor(pos.x - size.x / 2), glm::floor(pos.y),
                       glm::floor(pos.z + size.z / 2)};
        glm::ivec3 corner2 =
            glm::ivec3{glm::ceil(pos.x + size.x / 2), glm::ceil(pos.y + size.y),
                       glm::ceil(pos.z + size.z / 2 + 1)};

        return BlockRange{corner1, corner2 - corner1};
    }

    BlockRange getZNegBlockRange() const {
        glm::ivec3 corner1 =
            glm::ivec3{glm::floor(pos.x - size.x / 2), glm::floor(pos.y),
                       glm::floor(pos.z - size.z / 2 - 1)};
        glm::ivec3 corner2 =
            glm::ivec3{glm::ceil(pos.x + size.x / 2), glm::ceil(pos.y + size.y),
                       glm::ceil(pos.z - size.z / 2)};

        return BlockRange{corner1, corner2 - corner1};
    }

    BlockRange getXPosBlockRange() const {
        glm::ivec3 corner1 =
            glm::ivec3{glm::floor(pos.x + size.x / 2), glm::floor(pos.y),
                       glm::floor(pos.z - size.z / 2)};
        glm::ivec3 corner2 = glm::ivec3{glm::ceil(pos.x + size.x / 2 + 1),
                                        glm::ceil(pos.y + size.y),
                                        glm::ceil(pos.z + size.z / 2)};

        return BlockRange{corner1, corner2 - corner1};
    }

    BlockRange getXNegBlockRange() const {
        glm::ivec3 corner1 =
            glm::ivec3{glm::floor(pos.x - size.x / 2 - 1), glm::floor(pos.y),
                       glm::floor(pos.z - size.z / 2)};
        glm::ivec3 corner2 =
            glm::ivec3{glm::ceil(pos.x - size.x / 2), glm::ceil(pos.y + size.y),
                       glm::ceil(pos.z + size.z / 2)};

        return BlockRange{corner1, corner2 - corner1};
    }

    float getDistanceToBottom() const { return pos.y - glm::floor(pos.y); }
    float getDistanceToTop() const {
        return glm::ceil(pos.y + size.y) - (pos.y + size.y);
    }

    float getDistanceToZPos() const {
        return glm::ceil(pos.z + size.z / 2) - (pos.z + size.z / 2);
    }

    float getDistanceToZNeg() const {
        return (pos.z - size.z / 2) - glm::floor(pos.z - size.z / 2);
    }

    float getDistanceToXPos() const {
        return glm::ceil(pos.x + size.x / 2) - (pos.x + size.x / 2);
    }

    float getDistanceToXNeg() const {
        return (pos.x - size.x / 2) - glm::floor(pos.x - size.x / 2);
    }
};

class SimulatedBoxCollider {
public:
    static constexpr glm::vec3 GRAVITY{0.0f, -0.0005f, 0.0f};
    static constexpr float DRAG{0.002f};

    static constexpr glm::vec3 MAX_SPEED{0.05f, 0.05f, 0.05f};

    SimulatedBoxCollider(glm::vec3 pos, glm::vec3 size)
        : pos{pos}, prevPos{pos}, size{size} {}

    void update(world::World &world, glm::vec3 acc);
    void teleport(glm::vec3 newPos);
    void unstuck(world::World &world);

    glm::vec3 computeAccForSpeed(glm::vec3 speed);

    BoxCollider getCollider() const { return BoxCollider{pos, size}; }
    glm::vec3 getPos() const { return pos; }
    bool isOnGround() const { return onGround; }
    bool isAgainstWall() const { return againstWall; }

private:
    static bool checkCollision(const BlockRange &range, world::World &world);

    glm::vec3 pos;
    glm::vec3 prevPos;
    glm::vec3 size;

    bool onGround{false};
    bool againstWall{false};
};

}  // namespace logic