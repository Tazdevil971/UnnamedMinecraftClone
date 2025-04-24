#pragma once

#include "../logic/Collision.hpp"
#include "AnimModel.hpp"
#include "World.hpp"

namespace world {

class Mucchina {
public:
    static constexpr int ACTION_TIMER_REFILL = 2000;
    static constexpr float ANIM_PER_TICK = 0.005;
    static constexpr float JUMP = 0.04f;
    static constexpr float SPEED = 0.01f;
    static constexpr float TARGET_AREA = 1.0f;

    Mucchina();

    void cleanup() { model.cleanup(); }

    void update(World& world);
    void teleport(glm::vec3 newPos) { collider.teleport(newPos); }
    void unstuck(world::World& world) { collider.unstuck(world); }

    void setRot(glm::quat rot) { model.setRot(rot); }
    void setPos(glm::vec3 pos) { model.setPos(pos); }
    void addToModelList(std::list<render::GeometryModel>& models) {
        model.addToModelList(models);
    }

private:
    void updateDir();
    void updateWalkAnim();

    int actionTimer{0};
    float animTs{0.0f};

    glm::vec3 target;

    AnimModel model;
    logic::SimulatedBoxCollider collider;

    AnimModel::JointId body;
    AnimModel::JointId frontLeftLeg;
    AnimModel::JointId frontRightLeg;
    AnimModel::JointId backLeftLeg;
    AnimModel::JointId backRightLeg;
    AnimModel::JointId head;
};

}  // namespace world