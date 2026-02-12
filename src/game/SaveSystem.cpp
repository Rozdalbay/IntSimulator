#include "game/SaveSystem.h"
#include "core/Logger.h"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace civ {

bool SaveSystem::saveGame(const Civilization& civ, const EventSystem& events,
                          Difficulty difficulty, const std::string& filename) {
    try {
        std::ofstream file(filename, std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            m_lastError = "Cannot open file for writing: " + filename;
            Logger::instance().error(m_lastError);
            return false;
        }

        file << SAVE_HEADER << "\n";
        file << static_cast<int>(difficulty) << "\n";
        file << civ.serialize() << "\n";
        file << events.serialize() << "\n";
        file << "END_SAVE\n";

        file.close();
        Logger::instance().info("Game saved to: " + filename);
        return true;
    }
    catch (const std::exception& e) {
        m_lastError = std::string("Save failed: ") + e.what();
        Logger::instance().error(m_lastError);
        return false;
    }
}

bool SaveSystem::loadGame(Civilization& civ, EventSystem& events,
                          Difficulty& difficulty, const std::string& filename) {
    try {
        std::ifstream file(filename, std::ios::in);
        if (!file.is_open()) {
            m_lastError = "Cannot open file for reading: " + filename;
            Logger::instance().error(m_lastError);
            return false;
        }

        std::string header;
        std::getline(file, header);
        if (header != SAVE_HEADER) {
            m_lastError = "Invalid save file format";
            Logger::instance().error(m_lastError);
            return false;
        }

        int diff;
        file >> diff;
        difficulty = static_cast<Difficulty>(diff);
        file.ignore(1); // newline

        std::string civData;
        std::getline(file, civData);
        civ.deserialize(civData);

        std::string eventData;
        std::getline(file, eventData);
        events.deserialize(eventData);

        file.close();
        Logger::instance().info("Game loaded from: " + filename);
        return true;
    }
    catch (const std::exception& e) {
        m_lastError = std::string("Load failed: ") + e.what();
        Logger::instance().error(m_lastError);
        return false;
    }
}

bool SaveSystem::saveExists(const std::string& filename) {
    return std::filesystem::exists(filename);
}

bool SaveSystem::deleteSave(const std::string& filename) {
    try {
        if (std::filesystem::exists(filename)) {
            return std::filesystem::remove(filename);
        }
        return false;
    }
    catch (...) {
        return false;
    }
}

} // namespace civ
