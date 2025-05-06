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
    glm::vec3 lightDir;

    glm::mat4 computeModelMat() const {
        return glm::toMat4(
            glm::quatLookAt(lightDir, glm::vec3{0.0f, 1.0f, 0.0f}));
    }
};

}  // namespace render