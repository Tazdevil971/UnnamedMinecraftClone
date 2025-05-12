#include <iostream>

#include "glm/glm.hpp"

static inline void printVec(glm::vec3 vec) {
    std::cout << "x: " << vec.x << " y: " << vec.y << " z: " << vec.z;
}

static inline void printVec(glm::ivec3 vec) {
    std::cout << "x: " << vec.x << " y: " << vec.y << " z: " << vec.z;
}