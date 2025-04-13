#include "DayNightCycle.hpp"

#include <cmath>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace logic;

// 1 day lasts a minute
constexpr float DAY_DURATION{60.0f};

constexpr glm::vec3 STARTING_SUN_DIR{0.1, 0.0, 0.5};

constexpr glm::vec3 DAY_AMBIENT{0.5f, 0.4f, 0.4f};
constexpr glm::vec3 DAY_SUN{0.6f, 0.5f, 0.5f};

constexpr glm::vec3 NIGHT_AMBIENT{0.1f, 0.4f, 0.5f};
constexpr glm::vec3 NIGHT_SUN{0.0f, 0.0f, 0.0f};

glm::vec3 logic::computeSunDir(float angle) {
    // Start from a random sun direction on the XZ plane
    glm::vec3 sunDir = glm::normalize(STARTING_SUN_DIR);

    // Compute the rotation vector, to rotate around
    glm::vec3 sunRotateVec = glm::cross(sunDir, glm::vec3{0.0, 1.0, 0.0});

    // Finally rotate sun
    return glm::rotate(sunDir, angle, sunRotateVec);
}

DayNightState logic::getDayNightState(float time) {
    float cycle = std::fmod(time, DAY_DURATION * 2) / DAY_DURATION;

    float sunAngle = cycle * M_PI;
    glm::vec3 sunDir = computeSunDir(sunAngle);

    float fade;
    if (cycle < 0.2f) {
        fade = cycle / 0.2f;
    } else if (cycle < 0.8f) {
        fade = 1.0f;
    } else if (cycle < 1.0f) {
        fade = 1.0f - ((cycle - 0.8f) / 0.2f);
    } else {
        fade = 0.0f;
    }

    glm::vec3 ambientColor = glm::mix(NIGHT_AMBIENT, DAY_AMBIENT, fade);
    glm::vec3 sunColor = glm::mix(NIGHT_SUN, DAY_SUN, fade);
    glm::quat skyboxRot = glm::rotation(glm::vec3(0.0f, 1.0f, 0.0f), sunDir);

    return {ambientColor, sunDir, sunColor, skyboxRot, fade};
}