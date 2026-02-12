# 🏛️ Civilization Evolution Simulator

A console-based C++17 simulation game where you guide a civilization from the **Stone Age** to the **Space Age** through resource management, technology research, and surviving random events.

## 📋 Features

- **9 Historical Eras**: Stone Age → Bronze Age → Iron Age → Medieval → Renaissance → Industrial → Modern → Information → Space
- **Resource Management**: Food, Money, Energy, Materials
- **Technology Tree**: 5 branches (Science, Medicine, Military, Industry, Space) with 30+ technologies
- **Random Events**: Epidemics, Wars, Natural Disasters, Golden Ages, Revolutions, and more
- **4 Difficulty Levels**: Easy, Normal, Hard, Nightmare
- **Save/Load System**: Persist your game progress
- **Colored ASCII Interface**: Rich console UI with ANSI colors
- **Event Logging**: Detailed log file and in-game event history

## 🏆 Victory Conditions

| Condition | Requirement |
|-----------|-------------|
| 🚀 Space Colonization | Max out the Space tech branch |
| 💰 Economic Stability | Maintain stable economy for 50 consecutive turns |
| 🔬 Tech Supremacy | Reach overall tech level 100 |

## 💀 Defeat Conditions

| Condition | Trigger |
|-----------|---------|
| ☠️ Extinction | Population reaches 0 |
| 🌍 Ecological Collapse | Ecology drops below 5% |
| 📉 Economic Collapse | Bankrupt with no food |

## 🔧 Build Instructions

### Prerequisites

- C++17 compatible compiler (GCC 8+, Clang 7+, MSVC 2017+)
- CMake 3.16+

### Build with CMake

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build . --config Release

# Run
./IntSimulator        # Linux/macOS
IntSimulator.exe      # Windows
```

### Build with g++ (without CMake)

```bash
g++ -std=c++17 -O2 -Wall -Wextra -Iinclude \
    src/main.cpp \
    src/core/Logger.cpp \
    src/core/ColorOutput.cpp \
    src/core/Utils.cpp \
    src/game/Civilization.cpp \
    src/game/ResourceManager.cpp \
    src/game/TechnologyTree.cpp \
    src/game/EventSystem.cpp \
    src/game/GameEngine.cpp \
    src/game/SaveSystem.cpp \
    src/ui/Display.cpp \
    src/ui/InputHandler.cpp \
    -o IntSimulator
```

### Build with MSVC (Developer Command Prompt)

```cmd
cl /std:c++17 /EHsc /O2 /Iinclude /Fe:IntSimulator.exe ^
    src\main.cpp ^
    src\core\Logger.cpp ^
    src\core\ColorOutput.cpp ^
    src\core\Utils.cpp ^
    src\game\Civilization.cpp ^
    src\game\ResourceManager.cpp ^
    src\game\TechnologyTree.cpp ^
    src\game\EventSystem.cpp ^
    src\game\GameEngine.cpp ^
    src\game\SaveSystem.cpp ^
    src\ui\Display.cpp ^
    src\ui\InputHandler.cpp
```

## 📁 Project Structure

```
IntSimulator/
├── CMakeLists.txt
├── README.md
├── .gitignore
├── include/
│   ├── core/
│   │   ├── Types.h          # Enums, constants, type definitions
│   │   ├── Logger.h          # Singleton logger
│   │   ├── ColorOutput.h     # ANSI color console output
│   │   └── Utils.h           # Random, formatting, helpers
│   ├── game/
│   │   ├── Civilization.h     # Core civilization entity
│   │   ├── ResourceManager.h  # Resource system
│   │   ├── TechnologyTree.h   # Tech tree with 5 branches
│   │   ├── EventSystem.h      # Random event generation
│   │   ├── GameEngine.h       # Main game loop orchestrator
│   │   └── SaveSystem.h       # Save/load functionality
│   └── ui/
│       ├── Display.h          # Console rendering
│       └── InputHandler.h     # Input validation
└── src/
    ├── main.cpp               # Entry point
    ├── core/
    │   ├── Logger.cpp
    │   ├── ColorOutput.cpp
    │   └── Utils.cpp
    ├── game/
    │   ├── Civilization.cpp
    │   ├── ResourceManager.cpp
    │   ├── TechnologyTree.cpp
    │   ├── EventSystem.cpp
    │   ├── GameEngine.cpp
    │   └── SaveSystem.cpp
    └── ui/
        ├── Display.cpp
        └── InputHandler.cpp
```

## 🎮 Gameplay

Each turn you can:
1. **Advance Turn** - Process resources, population growth, and trigger a random event
2. **Invest Resources** - Spend money to advance technology branches
3. **Research Technology** - Unlock specific technologies when branch levels are high enough
4. **View Status** - See detailed civilization statistics
5. **View Tech Tree** - Browse available and researched technologies
6. **View Event Log** - Review past events
7. **Save Game** - Persist current progress

## 🏗️ Architecture

- **C++17** with STL containers and algorithms
- **SOLID principles** with single-responsibility classes
- **Smart pointers** (`std::unique_ptr`) for memory management
- **Exception handling** throughout
- **Thread-safe logging** with mutex protection
- **Clean separation** of concerns: core/game/ui layers

## 📜 License

MIT License
# IntSimulator
