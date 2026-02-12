#pragma once

#include "game/Civilization.h"
#include "game/EventSystem.h"
#include "game/SaveSystem.h"
#include "ui/Display.h"
#include "ui/InputHandler.h"
#include "core/Types.h"
#include <memory>

namespace civ {

/**
 * @brief Main game engine orchestrating the game loop.
 *        Coordinates all subsystems: civilization, events, display, save.
 */
class GameEngine {
public:
    GameEngine();
    ~GameEngine();

    // Non-copyable
    GameEngine(const GameEngine&) = delete;
    GameEngine& operator=(const GameEngine&) = delete;

    // Main entry point
    void run();

private:
    // Game state
    std::unique_ptr<Civilization> m_civ;
    std::unique_ptr<EventSystem> m_events;
    std::unique_ptr<SaveSystem> m_saveSystem;
    std::unique_ptr<Display> m_display;

    Difficulty m_difficulty = Difficulty::Normal;
    bool m_running = false;
    GameResult m_result = GameResult::InProgress;

    // Game phases
    void showMainMenu();
    void startNewGame();
    void loadGame();
    void gameLoop();
    void processTurn();
    void handlePlayerAction();
    void handleInvestment();
    void handleTechResearch();
    void handleSaveGame();
    void checkEndConditions();
    void showEndScreen();

    // Initialization
    void initSystems();
    void cleanup();
};

} // namespace civ
