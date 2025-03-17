#include <iostream>
#include <stdexcept>

#include "PlayerController.hpp"
#include "VoxelRaytracer.hpp"
#include "backward.hpp"

backward::SignalHandling sh;

int main() {
    std::unique_ptr<PlayerController> player;
    player = std::make_unique<PlayerController>();
    player->mainLoop();
    player->cleanup();
    return 0;
}