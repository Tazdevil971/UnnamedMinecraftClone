#include <cstdio>

#include "Window.hpp"

int main() {

    Window window;
    window.mainLoop();
    window.cleanup();

    return 0;
}