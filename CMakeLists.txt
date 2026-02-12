cmake_minimum_required(VERSION 3.16)
project(IntSimulator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Console version sources
set(CONSOLE_SOURCES
    src/main.cpp
    src/core/Logger.cpp
    src/core/ColorOutput.cpp
    src/core/Utils.cpp
    src/game/Civilization.cpp
    src/game/ResourceManager.cpp
    src/game/TechnologyTree.cpp
    src/game/EventSystem.cpp
    src/game/GameEngine.cpp
    src/game/SaveSystem.cpp
    src/ui/Display.cpp
    src/ui/InputHandler.cpp
)

# GUI version sources
set(GUI_SOURCES
    src/main_gui.cpp
    src/ui/Win32Gui.cpp
    src/core/Logger.cpp
    src/core/ColorOutput.cpp
    src/core/Utils.cpp
    src/game/Civilization.cpp
    src/game/ResourceManager.cpp
    src/game/TechnologyTree.cpp
    src/game/EventSystem.cpp
    src/game/SaveSystem.cpp
)

# Header files
set(HEADERS
    include/core/Logger.h
    include/core/ColorOutput.h
    include/core/Utils.h
    include/core/Types.h
    include/game/Civilization.h
    include/game/ResourceManager.h
    include/game/TechnologyTree.h
    include/game/EventSystem.h
    include/game/GameEngine.h
    include/game/SaveSystem.h
    include/ui/Display.h
    include/ui/InputHandler.h
    include/ui/Win32Gui.h
)

# Console executable
add_executable(IntSimulator ${CONSOLE_SOURCES} ${HEADERS})
target_include_directories(IntSimulator PRIVATE ${CMAKE_SOURCE_DIR}/include)

# GUI executable (Windows subsystem)
if(WIN32)
    add_executable(IntSimulatorGUI WIN32 ${GUI_SOURCES} ${HEADERS})
    target_include_directories(IntSimulatorGUI PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(IntSimulatorGUI comctl32)
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(IntSimulator PRIVATE /W4 /utf-8)
    if(WIN32)
        target_compile_options(IntSimulatorGUI PRIVATE /W4 /utf-8)
    endif()
else()
    target_compile_options(IntSimulator PRIVATE -Wall -Wextra -Wpedantic)
    if(WIN32)
        target_compile_options(IntSimulatorGUI PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()

# Install
install(TARGETS IntSimulator DESTINATION bin)
if(WIN32)
    install(TARGETS IntSimulatorGUI DESTINATION bin)
endif()
