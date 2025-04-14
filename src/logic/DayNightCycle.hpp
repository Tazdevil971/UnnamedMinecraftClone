#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace logic {

struct DayNightState {
    glm::vec3 ambientColor;
    glm::vec3 sunDir;
    glm::vec3 sunColor;
    glm::quat skyboxRot;
    float skyboxFade;
};

DayNightState getDayNightState(float time);

}  // namespace logic