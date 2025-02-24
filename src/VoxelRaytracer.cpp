#include "VoxelRaytracer.hpp"

#include <iostream>

using namespace utils;

VoxelRaytracer::VoxelRaytracer(glm::vec3 start, glm::vec3 dir) {
    // Convert start into integer position
    pos = glm::floor(start);
    dir = glm::normalize(dir);

    deltaDist = {
        dir.x == 0.0f ? INFINITY : 1.0f / std::abs(dir.x),
        dir.y == 0.0f ? INFINITY : 1.0f / std::abs(dir.y),
        dir.z == 0.0f ? INFINITY : 1.0f / std::abs(dir.z)
    };
    step = {
        (dir.x > 0) ? 1 : -1,
        (dir.y > 0) ? 1 : -1,
        (dir.z > 0) ? 1 : -1,
    };

    sideDist = {
        (dir.x > 0) ? (pos.x + 1 - start.x) * deltaDist.x : (start.x - pos.x) * deltaDist.x,
        (dir.y > 0) ? (pos.y + 1 - start.y) * deltaDist.y : (start.y - pos.y) * deltaDist.y,
        (dir.z > 0) ? (pos.z + 1 - start.z) * deltaDist.z : (start.z - pos.z) * deltaDist.z,
    };

    sideDist = {
        dir.x == 0.0f ? INFINITY : sideDist.x,
        dir.y == 0.0f ? INFINITY : sideDist.y,
        dir.z == 0.0f ? INFINITY : sideDist.z,
    };

    std::cout << "step: " << step.x << " " << step.y << " " << step.z << std::endl;
    std::cout << "sideDist: " << sideDist.x << " " << sideDist.y << " " << sideDist.z << std::endl;
    std::cout << "deltaDist: " << deltaDist.x << " " << deltaDist.y << " " << deltaDist.z << std::endl;
}

VoxelRaytracer::Hit VoxelRaytracer::getNextHit() { 
    float dist;

    if (sideDist.x < sideDist.y && sideDist.x < sideDist.z) {
        dist = sideDist.x;
        sideDist.x += deltaDist.x;
        pos.x += step.x;
    } else if (sideDist.y < sideDist.z) {
        dist = sideDist.y;
        sideDist.y += deltaDist.y;
        pos.y += step.y;
    } else {
        dist = sideDist.z;
        sideDist.z += deltaDist.z;
        pos.z += step.z;
    }

    std::cout << "sideDist: " << sideDist.x << " " << sideDist.y << " " << sideDist.z << std::endl;
    std::cout << "pos: " << pos.x << " " << pos.y << " " << pos.z << std::endl;
    std::cout << "dist: " << dist << std::endl;

    return Hit { pos, pos, dist };
}