#pragma once

#include "game/Civilization.h"
#include "game/EventSystem.h"
#include "core/Types.h"
#include <string>
#include <vector>

namespace civ {

/**
 * @brief Handles all console display: ASCII art, status panels, menus.
 */
class Display {
public:
    Display() = default;

    // Screen rendering
    void showTitle() const;
    void showMainMenu() const;
    void showDifficultyMenu() const;
    void showGameStatus(const Civilization& civ) const;
    void showResourcePanel(const Civilization& civ) const;
    void showTechTree(const Civilization& civ) const;
    void showEvent(const GameEvent& event) const;
    void showTurnMenu(const Civilization& civ) const;
    void showInvestmentMenu(const Civilization& civ) const;
    void showEventLog(const EventSystem& events) const;
    void showVictory(GameResult result) const;
    void showDefeat(GameResult result) const;
    void showHelp() const;

    // Utility
    void clearScreen() const;
    void waitForInput() const;
    void showSeparator(int width = 60) const;
    void showDoubleSeparator(int width = 60) const;

    // ASCII art
    void showEraArt(Era era) const;

private:
    void showBar(const std::string& label, double value, double maxVal,
                 int width = 30, const std::string& color = "") const;
};

} // namespace civ
