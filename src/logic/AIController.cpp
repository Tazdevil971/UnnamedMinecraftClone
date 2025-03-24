#include "AIController.hpp"

#include <iostream>

#include "../Random.hpp"

using namespace logic;
using namespace world;

void AIController::update(World &world) {
    if (actionTimer > 0) actionTimer--;

    if (actionTimer == 0) {
        std::uniform_real_distribution<float> dist{-1.0f, 1.0f};

        // Update with new action
        target.x = collider.getPos().x + dist(RNG) * 2.0f;
        target.y = 0.0f;
        target.z = collider.getPos().z + dist(RNG) * 2.0f;

        actionTimer = ACTION_TIMER_REFILL;
    }

    glm::vec3 mov = collider.getPos() - target;
    mov.y = 0.0f;

    if (glm::length(mov) > SPEED) mov = glm::normalize(mov) * SPEED;

    glm::vec3 acc = collider.computeAccForSpeed(mov);
    if (collider.isOnGround() && collider.isAgainstWall()) {
        acc.y += JUMP;
    } else {
        acc.y = 0.0f;
    }

    collider.update(world, acc);
}