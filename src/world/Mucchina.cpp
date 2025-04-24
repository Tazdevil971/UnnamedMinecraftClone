#include "Mucchina.hpp"

#include <algorithm>
#include <cmath>

#include "../Random.hpp"
#include "AnimModel.hpp"
#include "glm/geometric.hpp"

using namespace world;

Mucchina::Mucchina()
    : model{"assets/debug.png", VK_FORMAT_R8G8B8A8_SRGB},
      collider{{0, 0, 0}, {0.75f, 1.25f, 0.75f}} {
    // Build mucchina!
    body = model.addJoint(AnimModel::JointId::root(), {12, 5, 6}, {0, 14, 0},
                          glm::vec3{0, 0, 0}, {24, 10, 12}, {0, 0});

    backLeftLeg = model.addJoint(body, {2, 14, 2}, {-10, 0, -4},
                                 glm::vec3{0, 0, 0}, {5, 9, 4}, {0, 0});
    backRightLeg = model.addJoint(body, {2, 14, 2}, {-10, 0, 4},
                                  glm::vec3{0, 0, 0}, {5, 9, 4}, {0, 0});

    frontLeftLeg = model.addJoint(body, {3, 14, 2}, {10, 0, -4},
                                  glm::vec3{0, 0, 0}, {5, 9, 4}, {0, 0});
    frontRightLeg = model.addJoint(body, {3, 14, 2}, {10, 0, 4},
                                   glm::vec3{0, 0, 0}, {5, 9, 4}, {0, 0});

    head = model.addJoint(body, {2, 2, 4}, {11, 4, 0}, glm::vec3{0, 0, 0},
                          {8, 8, 8}, {0, 0});
}

void Mucchina::update(World& world) {
    if (actionTimer == 0) {
        actionTimer = ACTION_TIMER_REFILL;

        std::uniform_real_distribution<float> dist{-1.0f, 1.0f};

        // Update with new action
        target.x = collider.getPos().x + dist(RNG) * 4.0f;
        target.y = 0.0f;
        target.z = collider.getPos().z + dist(RNG) * 4.0f;
    }
    actionTimer--;

    glm::vec3 acc{0.0f, 0.0f, 0.0f};

    // Compute move vector
    glm::vec3 mov = target - collider.getPos();
    mov.y = 0.0f;
    float dist = glm::length(mov);
    if (dist > TARGET_AREA) {
        if (dist > SPEED) {
            mov = glm::normalize(mov) * SPEED;
        }
    } else {
        mov = {0.0f, 0.0f, 0.0f};
    }

    // Computer acceleration
    acc = collider.computeAccForSpeed(mov);
    if (collider.isOnGround() && collider.isAgainstWall()) {
        acc.y += JUMP;
    } else {
        acc.y = 0.0f;
    }

    collider.update(world, acc);

    model.setPos(collider.getPos());
    updateDir();

    animTs = std::fmod(animTs + ANIM_PER_TICK, 1.0f);
    updateWalkAnim();
}

void Mucchina::updateDir() {
    glm::vec3 dir = collider.getSpeed();
    dir.y = 0.0f;

    if (glm::length(dir) < 0.00001) return;

    dir = glm::normalize(dir);

    const glm::vec3 FRONT{1.0f, 0.0f, 0.0f};

    glm::quat rot = glm::rotation(FRONT, dir);
    model.setRot(rot);
}

void Mucchina::updateWalkAnim() {
    float speed = glm::length(collider.getSpeed()) / SPEED;
    speed = std::clamp(speed, 0.0f, 1.0f);

    float angle1 = std::sin(animTs * M_PI * 2.0f) * speed * 0.5f;
    float angle2 = std::sin(animTs * M_PI * 2.0f + M_PI) * speed * 0.5f;

    model.setJointRot(backLeftLeg, glm::vec3{0, 0, angle1});
    model.setJointRot(frontRightLeg, glm::vec3{0, 0, angle1});
    model.setJointRot(backRightLeg, glm::vec3{0, 0, angle2});
    model.setJointRot(frontLeftLeg, glm::vec3{0, 0, angle2});
}