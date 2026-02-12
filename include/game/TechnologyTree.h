#pragma once

#include "core/Types.h"
#include <array>
#include <string>
#include <vector>

namespace civ {

/**
 * @brief Represents a single technology node in the tree.
 */
struct Technology {
    std::string name;
    TechBranch branch;
    int level;           // Required branch level to unlock
    int cost;            // Resource cost to research
    bool researched;
    std::string description;

    Technology() : branch(TechBranch::Science), level(0), cost(0), researched(false) {}
    Technology(std::string n, TechBranch b, int lvl, int c, std::string desc)
        : name(std::move(n)), branch(b), level(lvl), cost(c),
          researched(false), description(std::move(desc)) {}
};

/**
 * @brief Manages the technology tree with 5 branches.
 *        Each branch has levels that unlock new capabilities.
 */
class TechnologyTree {
public:
    TechnologyTree();

    // Investment
    void investInBranch(TechBranch branch, double amount);

    // Getters
    [[nodiscard]] int getBranchLevel(TechBranch branch) const;
    [[nodiscard]] int getOverallTechLevel() const;
    [[nodiscard]] double getBranchProgress(TechBranch branch) const;
    [[nodiscard]] double getBranchThreshold(TechBranch branch) const;
    [[nodiscard]] Era getCurrentEra() const;

    // Available technologies
    [[nodiscard]] std::vector<const Technology*> getAvailableTechs() const;
    [[nodiscard]] std::vector<const Technology*> getResearchedTechs() const;
    bool researchTech(const std::string& techName);

    // Bonuses from tech
    [[nodiscard]] double getProductionBonus() const;
    [[nodiscard]] double getMilitaryBonus() const;
    [[nodiscard]] double getMedicineBonus() const;
    [[nodiscard]] double getSpaceProgress() const;

    // Serialization
    [[nodiscard]] std::string serialize() const;
    void deserialize(const std::string& data);

    // Display
    [[nodiscard]] std::string getStatusString() const;

private:
    static constexpr size_t NUM_BRANCHES = static_cast<size_t>(TechBranch::COUNT);
    static constexpr int MAX_BRANCH_LEVEL = 20;

    std::array<int, NUM_BRANCHES> m_branchLevels{};
    std::array<double, NUM_BRANCHES> m_branchProgress{};
    std::vector<Technology> m_technologies;

    void initTechnologies();
    [[nodiscard]] double levelUpThreshold(int currentLevel) const;
    void checkLevelUp(TechBranch branch);
};

} // namespace civ
