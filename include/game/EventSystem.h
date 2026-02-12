#pragma once

#include "core/Types.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace civ {

// Forward declaration
class Civilization;

/**
 * @brief Represents a single game event with effects on civilization.
 */
struct GameEvent {
    std::string name;
    std::string description;
    EventType type;

    // Effects (can be positive or negative)
    int populationEffect = 0;          // Absolute change
    double populationMultiplier = 1.0; // Multiplicative change
    double happinessEffect = 0.0;
    double ecologyEffect = 0.0;
    double militaryEffect = 0.0;
    double economyEffect = 0.0;        // Money multiplier
    int techBoost = 0;                 // Tech level boost
    double foodEffect = 0.0;
    double energyEffect = 0.0;
    double materialsEffect = 0.0;

    int duration = 1; // How many turns the event lasts
};

/**
 * @brief Manages random events that affect the civilization.
 *        Events are weighted by era, difficulty, and current state.
 */
class EventSystem {
public:
    EventSystem();

    // Initialize event pool
    void init(Difficulty difficulty);

    // Generate a random event based on current state
    [[nodiscard]] GameEvent generateEvent(Era currentEra, int turn) const;

    // Get event history
    [[nodiscard]] const std::vector<GameEvent>& getEventHistory() const;
    void recordEvent(const GameEvent& event);

    // Difficulty modifier
    void setDifficulty(Difficulty difficulty);

    // Serialization
    [[nodiscard]] std::string serialize() const;
    void deserialize(const std::string& data);

private:
    Difficulty m_difficulty = Difficulty::Normal;
    std::vector<GameEvent> m_eventPool;
    std::vector<GameEvent> m_eventHistory;

    void buildEventPool();
    [[nodiscard]] double getDifficultyMultiplier() const;
    [[nodiscard]] std::vector<GameEvent> getEventsForEra(Era era) const;
};

} // namespace civ
