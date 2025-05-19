#include "Capretta.hpp"

#include <algorithm>
#include <cmath>

#include "../Random.hpp"
#include "AnimModel.hpp"
#include "glm/geometric.hpp"

using namespace world;
using namespace render;

CaprettaBlueprint::CaprettaBlueprint()
    : blueprint{"assets/capretta.png", VK_FORMAT_R8G8B8A8_SRGB} {
    body =
        blueprint.addJoint(AnimModelBlueprint::JointId::root(), {6, 3, 3},
                           {0, 8, 0}, glm::vec3{0, 0, 0}, {12, 6, 6}, {0, 0});

    backLeftLeg = blueprint.addJoint(body, {1, 3, 1}, {-4, -6, -2},
                                     glm::vec3{0, 0, 0}, {2, 6, 2}, {0, 12});
    backRightLeg = blueprint.addJoint(body, {1, 3, 1}, {-4, -6, 2},
                                      glm::vec3{0, 0, 0}, {2, 6, 2}, {0, 12});
    frontLeftLeg = blueprint.addJoint(body, {1, 3, 1}, {4, -6, -2},
                                      glm::vec3{0, 0, 0}, {2, 6, 2}, {0, 12});
    frontRightLeg = blueprint.addJoint(body, {1, 3, 1}, {4, -6, 2},
                                       glm::vec3{0, 0, 0}, {2, 6, 2}, {0, 12});

    head = blueprint.addJoint(body, {2, 2, 3}, {8, 3, 0}, glm::vec3{0, 0, 0},
                              {6, 6, 6}, {8, 12});
}

Capretta CaprettaBlueprint::fabricate() {
    return Capretta{shared_from_this(), blueprint.newPose()};
}

Capretta::Capretta(std::shared_ptr<CaprettaBlueprint> blueprint,
                   AnimModelPose&& pose)
    : blueprint{std::move(blueprint)},
      pose{std::move(pose)},
      collider{{0, 0, 0}, {0.75f, 1.25f, 0.75f}} {}

void Capretta::update(World& world) {
    if (actionTimer == 0) {
        actionTimer = ACTION_TIMER_REFILL;

        std::uniform_real_distribution<float> dist{-4.0f, 4.0f};

        // Update with new action
        target.x = collider.getPos().x + dist(RNG);
        target.y = 0.0f;
        target.z = collider.getPos().z + dist(RNG);
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

    pose.setPos(collider.getPos());
    updateDir();

    animTs = std::fmod(animTs + ANIM_PER_TICK, 1.0f);
    updateWalkAnim();
}

void Capretta::updateDir() {
    glm::vec3 dir = collider.getSpeed();
    dir.y = 0.0f;

    if (glm::length(dir) < 0.00001) return;

    dir = glm::normalize(dir);

    const glm::vec3 FRONT{1.0f, 0.0f, 0.0f};

    glm::quat rot = glm::rotation(FRONT, dir);
    pose.setRot(rot);
}

void Capretta::updateWalkAnim() {
    float speed = glm::length(collider.getSpeed()) / SPEED;
    speed = std::clamp(speed, 0.0f, 1.0f);

    float angle1 = std::sin(animTs * M_PI * 2.0f) * speed * 0.5f;
    float angle2 = std::sin(animTs * M_PI * 2.0f + M_PI) * speed * 0.5f;

    pose.setJointRot(blueprint->backLeftLeg, glm::vec3{0, 0, angle1});
    pose.setJointRot(blueprint->frontRightLeg, glm::vec3{0, 0, angle1});
    pose.setJointRot(blueprint->backRightLeg, glm::vec3{0, 0, angle2});
    pose.setJointRot(blueprint->frontLeftLeg, glm::vec3{0, 0, angle2});
}