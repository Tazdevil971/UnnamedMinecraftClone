#include "MainWindow.hpp"
#include "backward.hpp"

backward::SignalHandling sh;

int main() {
    MainWindow window{};
    window.mainLoop();
    return 0;
}