#include "Collision.hpp"

#include <iostream>

using namespace world;

void SimulatedBoxCollider::update(World &world, glm::vec3 acc) {
    acc += GRAVITY;

    glm::vec3 speed = pos - prevPos;
    std::cout << speed.x << " " << speed.y << " " << speed.z << std::endl;

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
        float dist = collider.getDistanceToXNeg();

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
        float dist = collider.getDistanceToXPos();

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
        float dist = collider.getDistanceToBottom();

        if (dist < -newVel.y &&
            checkCollision(collider.getBottomBlockRange(), world)) {
            // We are going to collide this frame
            newPos.y += -dist;
            onGround = true;
        } else {
            newPos.y += newVel.y;
        }
    }

    if (newVel.y > 0.0f) {
        auto collider = BoxCollider{newPos, size};
        float dist = collider.getDistanceToTop();

        if (dist < newVel.y &&
            checkCollision(collider.getTopBlockRange(), world)) {
            // We are going to collide this frame
            newPos.y += dist;
        } else {
            newPos.y += newVel.y;
        }
    }

    if (newVel.z < 0.0f) {
        auto collider = BoxCollider{newPos, size};
        float dist = collider.getDistanceToZNeg();

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
        float dist = collider.getDistanceToZPos();

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

/*
PhysicBox::PhysicBox(glm::vec3 pos, glm::vec3 size) : pos{pos}, prevPos{pos},
size{size} {}

void PhysicBox::update(World &world) {
    glm::vec3 acc = PhysicBox::GRAVITY;

    glm::vec3 newVel = pos - prevPos + acc;
    // Clamp max speed (to account for air drag for example)
    newVel = glm::clamp(newVel, -PhysicBox::MAX_SPEED, PhysicBox::MAX_SPEED);

    glm::vec3 newPos = pos;

    // Slowly integrate velocity
    while (glm::length(newVel) > 0.0f) {
        if (newVel.y < 0.0f) {
            auto [dist, collided] = getSafeDistanceBottom(newPos, world);
            dist = std::max(-dist, newVel.y);

            newPos.y += dist;
            newVel.y -= dist;


            if(collided)
                newVel.y = 0;
        }

        if (newVel.y > 0.0f) {
            newPos.y += newVel.y;
            newVel.y -= newVel.y;
        }
    }

    prevPos = pos;
    pos = newPos;
}

glm::vec3 PhysicBox::getPos() {
    return pos;
}

std::pair<float, bool> PhysicBox::getSafeDistanceBottom(glm::vec3 pos, World
&world) { int startX = glm::floor(pos.x - size.x / 2); int endX =
glm::ceil(pos.x + size.x / 2);

    int startZ = glm::floor(pos.z - size.z / 2);
    int endZ = glm::ceil(pos.z + size.z / 2);

    int y = glm::floor(pos.y) - 1;

    bool wouldCollide = false;

    for(int x = startX; x < endX; x++) {
        for(int z = startZ; z < endZ; z++) {
            if(world.getBlock({x, y, z}) != Block::AIR) {
                wouldCollide = true;
                break;
            }
        }

        if (wouldCollide)
            break;
    }

    if (wouldCollide) {
        return std::make_pair(pos.y - (y + 1), true);
    } else {
        return std::make_pair(1.0f, false);
    }
}
*/