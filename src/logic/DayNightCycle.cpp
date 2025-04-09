#include "DayNightCycle.hpp"

#include <cmath>
#include <glm/gtx/rotate_vector.hpp>

using namespace logic;
using namespace render;

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

Renderer::LightInfo logic::getDayNightState(float time) {
    float cycle = std::fmod(time, DAY_DURATION * 2) / DAY_DURATION;

    float sunAngle = cycle * M_PI;
    glm::vec3 sunDir = computeSunDir(sunAngle);

    glm::vec3 ambientColor;
    glm::vec3 sunColor;
    if (cycle < 0.3f) {
        // 0% - 100% day
        float mix = cycle / 0.3f;
        ambientColor = glm::mix(NIGHT_AMBIENT, DAY_AMBIENT, mix);
        sunColor = glm::mix(NIGHT_SUN, DAY_SUN, mix);
    } else if (cycle < 0.7f) {
        // Full day1
        ambientColor = DAY_AMBIENT;
        sunColor = DAY_SUN;
    } else if (cycle < 1.0f) {
        // 100% - 0% day
        float mix = (cycle - 0.7f) / 0.3f;
        ambientColor = glm::mix(DAY_AMBIENT, NIGHT_AMBIENT, mix);
        sunColor = glm::mix(DAY_SUN, NIGHT_SUN, mix);
    } else {
        // Full night
        ambientColor = NIGHT_AMBIENT;
        sunColor = NIGHT_SUN;
    }

    return {
        {ambientColor, 1.0f},
        {sunDir, 1.0f},
        {sunColor, 1.0f},
    };
}