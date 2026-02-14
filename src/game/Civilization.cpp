#include "game/Civilization.h"
#include "core/Utils.h"
#include "core/Logger.h"
#include "core/ColorOutput.h"
#include <sstream>
#include <algorithm>
#include <cmath>

namespace civ {

Civilization::Civilization(const std::string& name)
    : m_name(name)
    , m_population(INITIAL_POPULATION)
    , m_happiness(INITIAL_HAPPINESS)
    , m_ecology(INITIAL_ECOLOGY)
    , m_military(INITIAL_MILITARY)
    , m_turn(0)
    , m_stableEconomyTurns(0)
{
}

void Civilization::processTurn() {
    m_turn++;

    // Reset resource multipliers before events
    m_resources.resetMultipliers();

    // Process resources
    m_resources.processTurn(m_population, m_tech.getOverallTechLevel());

    // Population growth
    growPopulation();

    // Update ecology and happiness
    updateEcology();
    updateHappiness();

    // Track stable economy
    double money = m_resources.getResource(ResourceType::Money);
    double food = m_resources.getResource(ResourceType::Food);
    if (money > 0 && food > 0 && m_happiness > 50.0) {
        m_stableEconomyTurns++;
    } else {
        m_stableEconomyTurns = std::max(0, m_stableEconomyTurns - 1);
    }

    // Clamp values
    m_happiness = Utils::clamp(m_happiness, 0.0, 100.0);
    m_ecology = Utils::clamp(m_ecology, 0.0, 100.0);
    m_military = std::max(0.0, m_military);
    m_population = std::max(0, m_population);
}

void Civilization::applyEvent(const GameEvent& event) {
    // Population effects
    if (event.populationMultiplier != 1.0) {
        m_population = static_cast<int>(m_population * event.populationMultiplier);
    }
    m_population += event.populationEffect;
    m_population = std::max(0, m_population);

    // Attribute effects
    m_happiness += event.happinessEffect;
    m_ecology += event.ecologyEffect;
    m_military += event.militaryEffect;

    // Resource effects
    if (event.economyEffect != 0.0) {
        m_resources.addResource(ResourceType::Money, event.economyEffect);
    }
    if (event.foodEffect != 0.0) {
        m_resources.addResource(ResourceType::Food, event.foodEffect);
    }
    if (event.energyEffect != 0.0) {
        m_resources.addResource(ResourceType::Energy, event.energyEffect);
    }
    if (event.materialsEffect != 0.0) {
        m_resources.addResource(ResourceType::Materials, event.materialsEffect);
    }

    // Tech boost
    if (event.techBoost > 0) {
        // Distribute tech boost across all branches
        double perBranch = static_cast<double>(event.techBoost) * 20.0 /
                          static_cast<double>(TechBranch::COUNT);
        for (int i = 0; i < static_cast<int>(TechBranch::COUNT); ++i) {
            m_tech.investInBranch(static_cast<TechBranch>(i), perBranch);
        }
    }

    // Clamp values
    m_happiness = Utils::clamp(m_happiness, 0.0, 100.0);
    m_ecology = Utils::clamp(m_ecology, 0.0, 100.0);
    m_military = std::max(0.0, m_military);

    Logger::instance().info("Event applied: " + event.name);
}

Era Civilization::getCurrentEra() const {
    return m_tech.getCurrentEra();
}

GameResult Civilization::checkGameResult() const {
    // Defeat conditions
    if (m_population <= 0) {
        return GameResult::DefeatPopulation;
    }
    if (m_ecology <= ECOLOGY_COLLAPSE_THRESHOLD) {
        return GameResult::DefeatEcology;
    }
    if (m_resources.getResource(ResourceType::Money) <= ECONOMY_COLLAPSE_THRESHOLD &&
        m_resources.getResource(ResourceType::Food) <= 0) {
        return GameResult::DefeatEconomy;
    }

    // Victory conditions
    // Победа присуждается сразу при достижении Космической эры (уровень технологий >= 90)
    if (getCurrentEra() == Era::Space) {
        return GameResult::VictorySpace;
    }

    return GameResult::InProgress;
}

bool Civilization::isAlive() const {
    return m_population > 0;
}

void Civilization::growPopulation() {
    double rate = getGrowthRate();
    int growth = static_cast<int>(m_population * rate);
    m_population += growth;
    m_population = std::max(0, m_population);
}

double Civilization::getGrowthRate() const {
    double baseRate = 0.02; // 2% base growth

    // Food availability
    double food = m_resources.getResource(ResourceType::Food);
    double foodNeeded = m_population * 0.012;
    double foodRatio = (foodNeeded > 0) ? food / foodNeeded : 1.0;
    foodRatio = Utils::clamp(foodRatio, 0.0, 2.0);

    // Happiness factor
    double happinessFactor = m_happiness / 100.0;

    // Medicine bonus
    double medicineFactor = m_tech.getMedicineBonus();

    // Ecology factor
    double ecologyFactor = m_ecology / 100.0;

    double rate = baseRate * foodRatio * happinessFactor * medicineFactor * ecologyFactor;

    // Can be negative if conditions are terrible
    if (foodRatio < 0.5 || m_happiness < 20.0) {
        rate = std::min(rate, -0.01);
    }

    return Utils::clamp(rate, -0.05, 0.05);
}

void Civilization::updateEcology() {
    // Industry hurts ecology
    int industryLevel = m_tech.getBranchLevel(TechBranch::Industry);
    double industrialDamage = industryLevel * 0.15;

    // Science helps ecology (green tech)
    int scienceLevel = m_tech.getBranchLevel(TechBranch::Science);
    double scienceHelp = scienceLevel * 0.08;

    // Population pressure
    double popPressure = std::log10(std::max(1, m_population)) * 0.3;

    double ecologyChange = scienceHelp - industrialDamage - popPressure + 0.5;
    m_ecology += ecologyChange;
    m_ecology = Utils::clamp(m_ecology, 0.0, 100.0);
}

void Civilization::updateHappiness() {
    // Base happiness drift toward 50
    double drift = (50.0 - m_happiness) * 0.02;

    // Food surplus makes people happy
    double foodNet = m_resources.getNetIncome(ResourceType::Food);
    double foodHappiness = Utils::clamp(foodNet * 0.05, -5.0, 5.0);

    // Money surplus
    double moneyNet = m_resources.getNetIncome(ResourceType::Money);
    double moneyHappiness = Utils::clamp(moneyNet * 0.03, -3.0, 3.0);

    // Ecology
    double ecoHappiness = (m_ecology - 50.0) * 0.02;

    m_happiness += drift + foodHappiness + moneyHappiness + ecoHappiness;
    m_happiness = Utils::clamp(m_happiness, 0.0, 100.0);
}

void Civilization::modifyPopulation(int delta) {
    m_population += delta;
    m_population = std::max(0, m_population);
}

void Civilization::modifyHappiness(double delta) {
    m_happiness += delta;
    m_happiness = Utils::clamp(m_happiness, 0.0, 100.0);
}

void Civilization::modifyEcology(double delta) {
    m_ecology += delta;
    m_ecology = Utils::clamp(m_ecology, 0.0, 100.0);
}

void Civilization::modifyMilitary(double delta) {
    m_military += delta;
    m_military = std::max(0.0, m_military);
}

std::string Civilization::serialize() const {
    std::ostringstream oss;
    oss << m_name.size() << " " << m_name << " ";
    oss << m_population << " " << m_happiness << " " << m_ecology << " "
        << m_military << " " << m_turn << " " << m_stableEconomyTurns << " ";
    oss << "RES " << m_resources.serialize() << " ";
    oss << "TECH " << m_tech.serialize() << " ";
    return oss.str();
}

void Civilization::deserialize(const std::string& data) {
    std::istringstream iss(data);
    size_t nameLen;
    iss >> nameLen;
    iss.ignore(1);
    m_name.resize(nameLen);
    iss.read(&m_name[0], nameLen);

    iss >> m_population >> m_happiness >> m_ecology
        >> m_military >> m_turn >> m_stableEconomyTurns;

    std::string marker;
    iss >> marker; // "RES"
    // Read remaining for resources
    std::string resData;
    std::getline(iss, resData, 'T'); // Read until "TECH"
    m_resources.deserialize(resData);

    iss >> marker; // "ECH" (rest of TECH)
    std::string techData;
    std::getline(iss, techData);
    m_tech.deserialize(techData);
}

std::string Civilization::getStatusString() const {
    std::ostringstream oss;
    oss << u8"  Население:   " << Utils::formatNumber(m_population) << "\n";
    oss << u8"  Счастье:     " << Utils::progressBar(m_happiness, 100.0, 20)
        << " " << Utils::formatDouble(m_happiness) << "%\n";
    oss << u8"  Экология:    " << Utils::progressBar(m_ecology, 100.0, 20)
        << " " << Utils::formatDouble(m_ecology) << "%\n";
    oss << u8"  Армия:       " << Utils::formatDouble(m_military) << "\n";
    oss << u8"  Стаб. экон.: " << m_stableEconomyTurns << "/"
        << VICTORY_STABLE_ECONOMY_TURNS << u8" ходов\n";
    return oss.str();
}

} // namespace civ
