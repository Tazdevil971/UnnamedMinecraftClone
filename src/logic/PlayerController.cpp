#include "PlayerController.hpp"

#include <algorithm>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>

#include "VoxelRaytracer.hpp"

using namespace logic;
using namespace world;
using namespace render;

void PlayerController::update(World &world, const Window::InputState &input) {
    if (actionTimer > 0) actionTimer--;

    glm::vec3 mov{0.0f, 0.0f, 0.0f};

    if (input.forward) {
        mov.z = -1.0f;
    }
    if (input.backward) {
        mov.z = 1.0f;
    }

    if (input.left) {
        mov.x = -1.0f;
    }
    if (input.right) {
        mov.x = 1.0f;
    }

    // Normalize movement vector
    if (glm::length(mov) > 0.1) mov = glm::normalize(mov);

    // Rotate according to current view angle
    mov = glm::rotate(mov, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    // Scale according to delta time and speed
    mov *= SPEED;

    glm::vec3 acc = collider.computeAccForSpeed(mov);
    if (input.jump && collider.isOnGround()) {
        acc.y += JUMP;
    } else {
        acc.y = 0.0f;
    }

    collider.update(world, acc);

    glm::vec2 cursorDelta = (input.cursorPos - lastCursorPos) * SENSIBILITY;
    lastCursorPos = input.cursorPos;

    yaw += cursorDelta.x;
    pitch += cursorDelta.y;

    yaw = std::fmod(yaw, M_PI * 2.0f);
    // Clamp to prevent gimbal lock
    pitch = std::clamp(pitch, -(float)M_PI_2 + 1e-6f, (float)M_PI_2 - 1e-6f);

    if (!input.destroy && !input.place) actionTimer = 0;

    Camera camera = getCamera();

    Block block_map[10] = {Block::AIR,         Block::GRASS,    Block::DIRT,
                           Block::COBBLESTONE, Block::WOOD_LOG, Block::LEAF,
                           Block::CHERRY_LEAF, Block::DIAMOND,  Block::AIR,
                           Block::AIR};

    VoxelRaytracer tracer(camera.pos, camera.computeViewDir());
    for (int i = 0; i < MAX_RAY_DISTANCE; i++) {
        auto hit = tracer.getNextHit();
        auto block = world.getBlock(hit.pos);

        if (block != Block::AIR) {
            // We hit something
            if (input.destroy && actionTimer == 0) {
                world.updateBlock(hit.pos, Block::AIR);
                actionTimer = ACTION_TIMER_REFILL;
            }

            if (input.place && actionTimer == 0) {
                auto pos = hit.pos + hit.dir;
                if (!collider.getCollider().getBlockRange().isInside(pos)) {
                    actionTimer = ACTION_TIMER_REFILL;
                    world.updateBlock(pos, block_map[input.selected_block]);
                }
            }

            lookingAt = camera.pos + camera.computeViewDir() * hit.dist;
            break;
        }
    }
}
