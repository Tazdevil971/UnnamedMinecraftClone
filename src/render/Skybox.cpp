#include "Skybox.hpp"

#include "BufferManager.hpp"
#include "Primitives.hpp"

using namespace render;

// clang-format off
std::vector<uint16_t> SKYBOX_INDICES = {
    0,  1,  2,  0,  2,  3,
    4,  5,  6,  4,  6,  7,

    8,  9,  10, 8,  10, 11,
    12, 13, 14, 12, 14, 15,

    16, 17, 18, 16, 18, 19,
    20, 21, 22, 20, 22, 23,
};

std::vector<SkyboxVertex> SKYBOX_VERTICES = {
    // Z- side
    {{+1.0f, -1.0f, -1.0f}, {0.50f, 0.75f}},
    {{-1.0f, -1.0f, -1.0f}, {0.25f, 0.75f}},
    {{-1.0f, +1.0f, -1.0f}, {0.25f, 0.50f}},
    {{+1.0f, +1.0f, -1.0f}, {0.50f, 0.50f}},

    // Z+ side
    {{-1.0f, -1.0f, +1.0f}, {0.25f, 0.75f}},
    {{+1.0f, -1.0f, +1.0f}, {0.00f, 0.75f}},
    {{+1.0f, +1.0f, +1.0f}, {0.00f, 0.50f}},
    {{-1.0f, +1.0f, +1.0f}, {0.25f, 0.50f}},

    // Bottom
    {{-1.0f, -1.0f, -1.0f}, {0.50f, 0.25f}},
    {{+1.0f, -1.0f, -1.0f}, {0.25f, 0.25f}},
    {{+1.0f, -1.0f, +1.0f}, {0.25f, 0.00f}},
    {{-1.0f, -1.0f, +1.0f}, {0.50f, 0.00f}},

    // Top
    {{-1.0f, +1.0f, +1.0f}, {0.25f, 0.25f}},
    {{+1.0f, +1.0f, +1.0f}, {0.00f, 0.25f}},
    {{+1.0f, +1.0f, -1.0f}, {0.00f, 0.00f}},
    {{-1.0f, +1.0f, -1.0f}, {0.25f, 0.00f}},

    // X- side
    {{-1.0f, -1.0f, -1.0f}, {0.50f, 0.50f}},
    {{-1.0f, -1.0f, +1.0f}, {0.25f, 0.50f}},
    {{-1.0f, +1.0f, +1.0f}, {0.25f, 0.25f}},
    {{-1.0f, +1.0f, -1.0f}, {0.50f, 0.25f}},

    // X+ side
    {{+1.0f, -1.0f, +1.0f}, {0.25f, 0.50f}},
    {{+1.0f, -1.0f, -1.0f}, {0.00f, 0.50f}},
    {{+1.0f, +1.0f, -1.0f}, {0.00f, 0.25f}},
    {{+1.0f, +1.0f, +1.0f}, {0.25f, 0.25f}}
};
// clang-format on

Skybox Skybox::make(const std::string &daySkybox,
                    const std::string &nightSkybox) {
    Skybox skybox{};

    skybox.mesh = BufferManager::get().allocateMesh<SkyboxMesh>(
        SKYBOX_INDICES, SKYBOX_VERTICES);
    skybox.dayTexture = BufferManager::get().allocateTexture(
        daySkybox, VK_FORMAT_R8G8B8A8_SRGB);
    skybox.nightTexture = BufferManager::get().allocateTexture(
        nightSkybox, VK_FORMAT_R8G8B8A8_SRGB);
    return skybox;
}

void Skybox::cleanup() {
    BufferManager::get().deallocateMeshDefer(mesh);
    BufferManager::get().deallocateTextureDefer(dayTexture);
    BufferManager::get().deallocateTextureDefer(nightTexture);
}