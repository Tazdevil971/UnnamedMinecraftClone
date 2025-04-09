#pragma once

#include "../world/World.hpp"
#include "Collision.hpp"

namespace logic {

class AIController {
private:
    static constexpr int ACTION_TIMER_REFILL = 200 * 10;
    static constexpr int JUMP_TIMER_REFILL = 400;

    static constexpr float SPEED = 0.01f;
    static constexpr float JUMP = 0.04f;

public:
    AIController(glm::vec3 pos, glm::vec3 size)
        : collider{pos, size}, target{pos} {}

    void update(world::World &world);

    glm::vec3 getPos() const { return collider.getPos(); }

private:
    SimulatedBoxCollider collider;
    int actionTimer = 0;
    int jumpTimer = 0;
    glm::vec3 target{0.0f, 0.0f, 0.0f};
};

}  // namespace logic