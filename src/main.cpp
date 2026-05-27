#include "app/Application.h"
#include <iostream>

int main() {
    try {
        materializr::Application app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
