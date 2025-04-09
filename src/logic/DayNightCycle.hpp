#pragma once

#include <glm/glm.hpp>

#include "../render/Renderer.hpp"

namespace logic {

glm::vec3 computeSunDir(float angle);

render::Renderer::LightInfo getDayNightState(float time);

}  // namespace logic