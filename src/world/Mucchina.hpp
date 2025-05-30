#pragma once

#include <memory>

#include "../logic/Collision.hpp"
#include "AnimModel.hpp"
#include "World.hpp"

namespace world {

class Mucchina;

class MucchinaBlueprint
    : public std::enable_shared_from_this<MucchinaBlueprint> {
    friend class Mucchina;

public:
    MucchinaBlueprint();

    static std::shared_ptr<MucchinaBlueprint> create() {
        return std::make_shared<MucchinaBlueprint>();
    }

    Mucchina fabricate();

private:
    AnimModelBlueprint blueprint;
    AnimModelBlueprint::JointId body;
    AnimModelBlueprint::JointId frontLeftLeg;
    AnimModelBlueprint::JointId frontRightLeg;
    AnimModelBlueprint::JointId backLeftLeg;
    AnimModelBlueprint::JointId backRightLeg;
    AnimModelBlueprint::JointId head;
};

class Mucchina {
    friend class MucchinaBlueprint;

public:
    static constexpr int ACTION_TIMER_REFILL = 2000;
    static constexpr float ANIM_PER_TICK = 0.005f;
    static constexpr float JUMP = 0.04f;
    static constexpr float SPEED = 0.01f;
    static constexpr float TARGET_AREA = 1.0f;

    void update(World& world);
    void teleport(glm::vec3 newPos) { collider.teleport(newPos); }
    void unstuck(world::World& world) { collider.unstuck(world); }

    glm::vec3 getPos() const { return collider.getPos(); }
    glm::vec3 getSpeed() const { return collider.getSpeed(); }

    void addToModelList(std::list<render::GeometryModel>& models) {
        blueprint->blueprint.addToModelList(models, pose);
    }

private:
    Mucchina(std::shared_ptr<MucchinaBlueprint> blueprint,
             AnimModelPose&& pose);

    void updateDir();
    void updateWalkAnim();

    int actionTimer{0};
    float animTs{0.0f};

    glm::vec3 target;

    std::shared_ptr<MucchinaBlueprint> blueprint;
    AnimModelPose pose;
    logic::SimulatedBoxCollider collider;
};

}  // namespace world