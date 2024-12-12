#include <iostream>
#include <stdexcept>

#include "Window.hpp"

#include "backward.hpp"

backward::SignalHandling sh;

int main() {

    std::unique_ptr<Window> window;

    try {
        window = std::make_unique<Window>();
    } catch (std::exception &e) {
        std::cerr << "Initialization failure: " << e.what() << std::endl;
        return -1;
    }

    try {
        window->mainLoop();
    } catch (std::exception &e) {
        std::cerr << "Main loop failure: " << e.what() << std::endl;
        window->cleanup();
        return -1;
    }

    window->cleanup();
    return 0;
}