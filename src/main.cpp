#include <memory>

#include "MainWindow.hpp"
#include "backward.hpp"

backward::SignalHandling sh;

int main() {
    std::unique_ptr<MainWindow> window = std::make_unique<MainWindow>();
    window->mainLoop();
    window->cleanup();
    return 0;
}