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

DayNightState logic::getDayNightState(float time) {
    float cycle = std::fmod(time, DAY_DURATION * 2) / DAY_DURATION;

    float sunAngle = cycle * M_PI;
    glm::vec3 startingDir = glm::normalize(STARTING_SUN_DIR);
    glm::vec3 rotateNormal =
        glm::rotate(glm::vec3{0.0f, 1.0f, 0.0f}, 0.4f, startingDir);
    glm::vec3 rotateDir = glm::cross(startingDir, rotateNormal);

    glm::vec3 sunDir = glm::rotate(startingDir, sunAngle, rotateDir);

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

    return {ambientColor, sunDir, sunColor, fade};
}