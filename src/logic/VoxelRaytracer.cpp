#include "VoxelRaytracer.hpp"

using namespace logic;

VoxelRaytracer::VoxelRaytracer(glm::vec3 start, glm::vec3 dir) {
    // Convert start into integer position
    pos = glm::floor(start);
    dir = glm::normalize(dir);

    deltaDist = {dir.x == 0.0f ? INFINITY : 1.0f / std::abs(dir.x),
                 dir.y == 0.0f ? INFINITY : 1.0f / std::abs(dir.y),
                 dir.z == 0.0f ? INFINITY : 1.0f / std::abs(dir.z)};
    step = {
        (dir.x > 0) ? 1 : -1,
        (dir.y > 0) ? 1 : -1,
        (dir.z > 0) ? 1 : -1,
    };

    sideDist = {
        (dir.x > 0) ? (pos.x + 1 - start.x) * deltaDist.x
                    : (start.x - pos.x) * deltaDist.x,
        (dir.y > 0) ? (pos.y + 1 - start.y) * deltaDist.y
                    : (start.y - pos.y) * deltaDist.y,
        (dir.z > 0) ? (pos.z + 1 - start.z) * deltaDist.z
                    : (start.z - pos.z) * deltaDist.z,
    };

    sideDist = {
        dir.x == 0.0f ? INFINITY : sideDist.x,
        dir.y == 0.0f ? INFINITY : sideDist.y,
        dir.z == 0.0f ? INFINITY : sideDist.z,
    };
}

VoxelRaytracer::Hit VoxelRaytracer::getNextHit() {
    float dist;
    glm::ivec3 dir{0.0f, 0.0f, 0.0f};

    if (sideDist.x < sideDist.y && sideDist.x < sideDist.z) {
        dist = sideDist.x;
        dir.x = -step.x;

        sideDist.x += deltaDist.x;
        pos.x += step.x;
    } else if (sideDist.y < sideDist.z) {
        dist = sideDist.y;
        dir.y = -step.y;

        sideDist.y += deltaDist.y;
        pos.y += step.y;
    } else {
        dist = sideDist.z;
        dir.z = -step.z;

        sideDist.z += deltaDist.z;
        pos.z += step.z;
    }

    return Hit{pos, dir, dist};
}