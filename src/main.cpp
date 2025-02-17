#include <iostream>
#include <stdexcept>

#include "PlayerController.hpp"
#include "backward.hpp"

backward::SignalHandling sh;

int main() {
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