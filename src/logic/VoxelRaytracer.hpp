#pragma once

#include <glm/glm.hpp>

namespace logic {

class VoxelRaytracer {
public:
    struct Hit {
        glm::ivec3 pos;
        glm::ivec3 dir;
        float dist;
    };

    VoxelRaytracer(glm::vec3 start, glm::vec3 dir);

    Hit getNextHit();

private:
    glm::ivec3 pos;
    glm::ivec3 step;
    glm::vec3 deltaDist;
    glm::vec3 sideDist;
};

}  // namespace logic
