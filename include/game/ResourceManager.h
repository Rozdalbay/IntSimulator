#pragma once

#include "core/Types.h"
#include <array>
#include <string>
#include <map>

namespace civ {

/**
 * @brief Manages all resources: Food, Money, Energy, Materials.
 *        Handles production, consumption, and allocation.
 */
class ResourceManager {
public:
    ResourceManager();

    // Getters
    [[nodiscard]] double getResource(ResourceType type) const;
    [[nodiscard]] double getProduction(ResourceType type) const;
    [[nodiscard]] double getConsumption(ResourceType type) const;
    [[nodiscard]] double getNetIncome(ResourceType type) const;

    // Setters / Modifiers
    void addResource(ResourceType type, double amount);
    void removeResource(ResourceType type, double amount);
    void setProduction(ResourceType type, double amount);
    void setConsumption(ResourceType type, double amount);

    // Multipliers (from tech, events, etc.)
    void applyProductionMultiplier(ResourceType type, double multiplier);
    void applyConsumptionMultiplier(ResourceType type, double multiplier);

    // Turn processing
    void processTurn(int population, int techLevel);

    // Allocation: player distributes surplus resources
    [[nodiscard]] double getTotalSurplus() const;

    // Serialization
    [[nodiscard]] std::string serialize() const;
    void deserialize(const std::string& data);

    // Display
    [[nodiscard]] std::string getStatusString() const;

    // Reset multipliers (called each turn before events)
    void resetMultipliers();

private:
    static constexpr size_t NUM_RESOURCES = static_cast<size_t>(ResourceType::COUNT);

    std::array<double, NUM_RESOURCES> m_resources{};
    std::array<double, NUM_RESOURCES> m_production{};
    std::array<double, NUM_RESOURCES> m_consumption{};
    std::array<double, NUM_RESOURCES> m_prodMultiplier{};
    std::array<double, NUM_RESOURCES> m_consMultiplier{};

    void updateProduction(int population, int techLevel);
    void updateConsumption(int population);
};

} // namespace civ
