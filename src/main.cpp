#include <iostream>
#include <stdexcept>

#include "PlayerController.hpp"
#include "VoxelRaytracer.hpp"
#include "backward.hpp"

backward::SignalHandling sh;

int main() {
    // glm::vec3 dir = {1.0f, 0.0f, 0.5f};
    // glm::vec3 pos = {0.4f, 0.4f, 0.4f};
// 
    // utils::VoxelRaytracer tracer({0.4f, 0.4f, 0.4f}, {1.0f, 0.0f, 0.5f});
// 
    // for(int i = 0; i < 10; i++) {
    //     auto hit2 = tracer.getNextHit();
    //     glm::vec3 hit = pos + dir * hit2.dist;
    //     std::cout << "hit: " << hit.x << " " << hit.y << " " << hit.z << std::endl; 
    // }

    std::unique_ptr<PlayerController> player;

    try {
        player = std::make_unique<PlayerController>();
    } catch (std::exception &e) {
        std::cerr << "Initialization failure: " << e.what() << std::endl;
        return -1;
    }

    try {
        player->mainLoop();
    } catch (std::exception &e) {
        std::cerr << "Main loop failure: " << e.what() << std::endl;
        player->cleanup();
        return -1;
    }

    player->cleanup();
    return 0;
}