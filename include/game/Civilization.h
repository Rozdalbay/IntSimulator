#pragma once

#include "core/Types.h"
#include "game/ResourceManager.h"
#include "game/TechnologyTree.h"
#include "game/EventSystem.h"
#include <string>
#include <memory>

namespace civ {

/**
 * @brief Core entity representing the player's civilization.
 *        Aggregates all subsystems: resources, technology, events.
 */
class Civilization {
public:
    explicit Civilization(const std::string& name = "Humanity");

    // --- Core attributes ---
    [[nodiscard]] const std::string& getName() const { return m_name; }
    [[nodiscard]] int getPopulation() const { return m_population; }
    [[nodiscard]] double getHappiness() const { return m_happiness; }
    [[nodiscard]] double getEcology() const { return m_ecology; }
    [[nodiscard]] double getMilitary() const { return m_military; }
    [[nodiscard]] int getTurn() const { return m_turn; }
    [[nodiscard]] int getStableEconomyTurns() const { return m_stableEconomyTurns; }

    void setName(const std::string& name) { m_name = name; }

    // --- Subsystems ---
    [[nodiscard]] ResourceManager& getResources() { return m_resources; }
    [[nodiscard]] const ResourceManager& getResources() const { return m_resources; }
    [[nodiscard]] TechnologyTree& getTech() { return m_tech; }
    [[nodiscard]] const TechnologyTree& getTech() const { return m_tech; }

    // --- Turn processing ---
    void processTurn();
    void applyEvent(const GameEvent& event);

    // --- State queries ---
    [[nodiscard]] Era getCurrentEra() const;
    [[nodiscard]] GameResult checkGameResult() const;
    [[nodiscard]] bool isAlive() const;

    // --- Population management ---
    void growPopulation();
    void modifyPopulation(int delta);
    void modifyHappiness(double delta);
    void modifyEcology(double delta);
    void modifyMilitary(double delta);

    // --- Serialization ---
    [[nodiscard]] std::string serialize() const;
    void deserialize(const std::string& data);

    // --- Display ---
    [[nodiscard]] std::string getStatusString() const;

private:
    std::string m_name;
    int m_population;
    double m_happiness;     // 0-100
    double m_ecology;       // 0-100
    double m_military;      // 0+
    int m_turn;
    int m_stableEconomyTurns;

    ResourceManager m_resources;
    TechnologyTree m_tech;

    void updateEcology();
    void updateHappiness();
    [[nodiscard]] double getGrowthRate() const;
};

} // namespace civ
