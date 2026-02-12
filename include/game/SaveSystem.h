#pragma once

#include "game/Civilization.h"
#include "game/EventSystem.h"
#include "core/Types.h"
#include <string>
#include <memory>

namespace civ {

/**
 * @brief Handles saving and loading game state to/from files.
 */
class SaveSystem {
public:
    SaveSystem() = default;

    // Save/Load
    bool saveGame(const Civilization& civ, const EventSystem& events,
                  Difficulty difficulty, const std::string& filename = "savegame.dat");
    bool loadGame(Civilization& civ, EventSystem& events,
                  Difficulty& difficulty, const std::string& filename = "savegame.dat");

    // Check if save exists
    [[nodiscard]] static bool saveExists(const std::string& filename = "savegame.dat");

    // Delete save
    static bool deleteSave(const std::string& filename = "savegame.dat");

    // Get last error
    [[nodiscard]] const std::string& getLastError() const { return m_lastError; }

private:
    std::string m_lastError;

    static constexpr const char* SAVE_HEADER = "CIVSIM_SAVE_V1";
};

} // namespace civ
