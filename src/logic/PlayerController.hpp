#pragma once

#include "../render/Primitives.hpp"
#include "../render/Window.hpp"
#include "../world/World.hpp"
#include "Collision.hpp"

namespace logic {

class PlayerController {
public:
    static constexpr glm::vec3 SIZE{0.6f, 1.8f, 0.6f};
    static constexpr float CAMERA_HEIGHT = 1.4f;

    static constexpr float FOV = 45.0f;

    static constexpr float SPEED = 0.03f;
    static constexpr float JUMP = 0.04f;

    static constexpr float SENSIBILITY = 1e-3f;

    static constexpr int ACTION_TIMER_REFILL = 50;

    static constexpr int MAX_RAY_DISTANCE = 20;

    PlayerController(glm::vec3 pos)
        : collider{pos, SIZE}, yaw{0.0f}, pitch{0.0f} {}

    void update(world::World &world, const render::Window::InputState &input);

    glm::vec3 getPos() const { return collider.getPos(); }

    render::Camera getCamera() const {
        return render::Camera{
            FOV, 0.1f, 1000.0f,
            collider.getPos() + glm::vec3{0.0f, CAMERA_HEIGHT, 0.0f},
            glm::vec3{pitch, yaw, 0.0f}};
    }

    glm::vec3 getLookingAt() const { return lookingAt; }

private:
    SimulatedBoxCollider collider;
    int actionTimer{0};
    float yaw{0.0f};
    float pitch{0.0f};
    glm::vec2 lastCursorPos{0.0f, 0.0f};
    glm::vec3 lookingAt{0.0f, 0.0f, 0.0f};
};

}  // namespace logic