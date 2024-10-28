#include <iostream>
#include <stdexcept>

#include "Window.hpp"

int main() {
    Window window;

    try {
        window.init();
    } catch(std::exception &e) {
        std::cerr << "Initialization failure: " << e.what() << std::endl;
        window.cleanup();
        return -1;
    }

    try {
        window.mainLoop();
    } catch(std::exception &e) {
        std::cerr << "Main loop failure: " << e.what() << std::endl;
        window.cleanup();
        return -1;
    }
    
    window.cleanup();
    return 0;
}