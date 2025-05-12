#include "Collision.hpp"

using namespace logic;
using namespace world;

void SimulatedBoxCollider::update(World &world, glm::vec3 acc) {
    const float EPSILON = 0.0001;

    glm::vec3 speed = pos - prevPos;

    acc += GRAVITY;
    acc -= speed * glm::abs(speed) * DRAG;

    glm::vec3 newVel = speed + acc;

    // Clamp max speed
    newVel = glm::clamp(newVel, {-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f});

    glm::vec3 newPos = pos;
    onGround = false;
    againstWall = false;

    // ASSUMPTION: We ALWAYS move slower than a block/update
    if (newVel.x < 0.0f) {
        auto collider = BoxCollider{newPos, size};
        float dist = std::max(0.0f, collider.getDistanceToXNeg() - EPSILON);

        if (dist < -newVel.x &&
            checkCollision(collider.getXNegBlockRange(), world)) {
            // We are going to collide this frame
            newPos.x += -dist;
            againstWall = true;
        } else {
            newPos.x += newVel.x;
        }
    }

    if (newVel.x > 0.0f) {
        auto collider = BoxCollider{newPos, size};
        float dist = std::max(0.0f, collider.getDistanceToXPos() - EPSILON);

        if (dist < newVel.x &&
            checkCollision(collider.getXPosBlockRange(), world)) {
            // We are going to collide this frame
            newPos.x += dist;
            againstWall = true;
        } else {
            newPos.x += newVel.x;
        }
    }

    if (newVel.y < 0.0f) {
        auto collider = BoxCollider{newPos, size};
        float dist = std::max(0.0f, collider.getDistanceToYNeg() - EPSILON);

        if (dist < -newVel.y &&
            checkCollision(collider.getYNegBlockRange(), world)) {
            // We are going to collide this frame
            newPos.y += -dist;
            onGround = true;
        } else {
            newPos.y += newVel.y;
        }
    }

    if (newVel.y > 0.0f) {
        auto collider = BoxCollider{newPos, size};
        float dist = std::max(0.0f, collider.getDistanceToYPos() - EPSILON);

        if (dist < newVel.y &&
            checkCollision(collider.getYPosBlockRange(), world)) {
            // We are going to collide this frame
            newPos.y += dist;
        } else {
            newPos.y += newVel.y;
        }
    }

    if (newVel.z < 0.0f) {
        auto collider = BoxCollider{newPos, size};
        float dist = std::max(0.0f, collider.getDistanceToZNeg() - EPSILON);

        if (dist < -newVel.z &&
            checkCollision(collider.getZNegBlockRange(), world)) {
            // We are going to collide this frame
            newPos.z += -dist;
            againstWall = true;
        } else {
            newPos.z += newVel.z;
        }
    }

    if (newVel.z > 0.0f) {
        auto collider = BoxCollider{newPos, size};
        float dist = std::max(0.0f, collider.getDistanceToZPos() - EPSILON);

        if (dist < newVel.z &&
            checkCollision(collider.getZPosBlockRange(), world)) {
            // We are going to collide this frame
            newPos.z += dist;
            againstWall = true;
        } else {
            newPos.z += newVel.z;
        }
    }

    prevPos = pos;
    pos = newPos;
}

void SimulatedBoxCollider::teleport(glm::vec3 newPos) {
    pos = prevPos = newPos;
}

void SimulatedBoxCollider::unstuck(World &world) {
    glm::vec3 newPos = pos;
    auto collider = BoxCollider{newPos, size};

    if (checkCollision(collider.getBlockRange(), world)) {
        do {
            newPos += glm::vec3(0.0f, 1.0f, 0.0f);
            collider = BoxCollider{newPos, size};
        } while (checkCollision(collider.getBlockRange(), world));

        teleport(newPos);
    }
}

glm::vec3 SimulatedBoxCollider::computeAccForSpeed(glm::vec3 speed) {
    glm::vec3 curSpeed = pos - prevPos;
    return speed - curSpeed;
}

bool SimulatedBoxCollider::checkCollision(const BlockRange &range,
                                          World &world) {
    bool willCollide = false;
    range.visit([&](glm::ivec3 pos) {
        if (world.getBlock(pos) != Block::AIR) {
            willCollide = true;
            return false;
        } else {
            return true;
        }
    });
    return willCollide;
}