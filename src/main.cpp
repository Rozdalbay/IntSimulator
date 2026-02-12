#include "game/GameEngine.h"
#include "core/ColorOutput.h"
#include <iostream>
#include <exception>

/**
 * @brief Civilization Evolution Simulator
 * 
 * A console-based simulation game where the player guides a civilization
 * from the Stone Age to the Space Age through resource management,
 * technology research, and surviving random events.
 * 
 * Built with C++17, following SOLID principles and clean architecture.
 */
int main() {
    try {
        civ::GameEngine engine;
        engine.run();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown fatal error occurred." << std::endl;
        return 2;
    }
}
