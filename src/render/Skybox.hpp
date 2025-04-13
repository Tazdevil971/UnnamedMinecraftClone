#pragma once

#include <string>

#include "Primitives.hpp"

namespace render {

struct Skybox {
    static Skybox make(const std::string &path);

    SkyboxMesh mesh;
    Texture texture;
    glm::quat rot;

    void cleanup();

    glm::mat4 computeModelMat() const { return glm::toMat4(rot); }
};

}  // namespace render