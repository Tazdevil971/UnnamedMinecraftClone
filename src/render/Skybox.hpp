#pragma once

#include <string>

#include "Primitives.hpp"

namespace render {

struct Skybox {
    static Skybox make(const std::string &daySkybox,
                       const std::string &nightSkybox);

    SkyboxMesh mesh;
    Texture dayTexture;
    Texture nightTexture;
    float blend{0.0f};
    glm::quat rot;

    void cleanup();

    glm::mat4 computeModelMat() const { return glm::toMat4(rot); }
};

}  // namespace render