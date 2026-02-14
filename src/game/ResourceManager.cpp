#include "game/ResourceManager.h"
#include "core/Utils.h"
#include "core/ColorOutput.h"
#include <sstream>
#include <algorithm>
#include <cmath>

namespace civ {

ResourceManager::ResourceManager() {
    m_resources.fill(0.0);
    m_production.fill(0.0);
    m_consumption.fill(0.0);
    m_prodMultiplier.fill(1.0);
    m_consMultiplier.fill(1.0);

    // Starting resources
    m_resources[static_cast<size_t>(ResourceType::Food)]      = 500.0;
    m_resources[static_cast<size_t>(ResourceType::Money)]     = 200.0;
    m_resources[static_cast<size_t>(ResourceType::Energy)]    = 100.0;
    m_resources[static_cast<size_t>(ResourceType::Materials)] = 150.0;
}

double ResourceManager::getResource(ResourceType type) const {
    return m_resources[static_cast<size_t>(type)];
}

double ResourceManager::getProduction(ResourceType type) const {
    return m_production[static_cast<size_t>(type)] * m_prodMultiplier[static_cast<size_t>(type)];
}

double ResourceManager::getConsumption(ResourceType type) const {
    return m_consumption[static_cast<size_t>(type)] * m_consMultiplier[static_cast<size_t>(type)];
}

double ResourceManager::getNetIncome(ResourceType type) const {
    return getProduction(type) - getConsumption(type);
}

void ResourceManager::addResource(ResourceType type, double amount) {
    m_resources[static_cast<size_t>(type)] += amount;
}

void ResourceManager::removeResource(ResourceType type, double amount) {
    m_resources[static_cast<size_t>(type)] -= amount;
}

void ResourceManager::setProduction(ResourceType type, double amount) {
    m_production[static_cast<size_t>(type)] = amount;
}

void ResourceManager::setConsumption(ResourceType type, double amount) {
    m_consumption[static_cast<size_t>(type)] = amount;
}

void ResourceManager::applyProductionMultiplier(ResourceType type, double multiplier) {
    m_prodMultiplier[static_cast<size_t>(type)] *= multiplier;
}

void ResourceManager::applyConsumptionMultiplier(ResourceType type, double multiplier) {
    m_consMultiplier[static_cast<size_t>(type)] *= multiplier;
}

void ResourceManager::resetMultipliers() {
    m_prodMultiplier.fill(1.0);
    m_consMultiplier.fill(1.0);
}

void ResourceManager::processTurn(int population, int techLevel) {
    updateProduction(population, techLevel);
    updateConsumption(population);

    for (size_t i = 0; i < NUM_RESOURCES; ++i) {
        double net = getProduction(static_cast<ResourceType>(i))
                   - getConsumption(static_cast<ResourceType>(i));
        m_resources[i] += net;
        // Resources can go negative (debt) but floor at a reasonable limit
        m_resources[i] = std::max(m_resources[i], -10000.0);
    }
}

double ResourceManager::getTotalSurplus() const {
    double surplus = 0.0;
    for (size_t i = 0; i < NUM_RESOURCES; ++i) {
        double net = getNetIncome(static_cast<ResourceType>(i));
        if (net > 0) surplus += net;
    }
    return surplus;
}

void ResourceManager::updateProduction(int population, int techLevel) {
    // Изменено с sqrt на линейную зависимость, чтобы экономика не рушилась при росте населения
    double popFactor = static_cast<double>(population) * 0.01; 
    double techFactor = 1.0 + techLevel * 0.02;

    m_production[static_cast<size_t>(ResourceType::Food)]      = popFactor * 1.5 * techFactor; // База 1.5 > Потр 1.2
    m_production[static_cast<size_t>(ResourceType::Money)]     = popFactor * 1.1 * techFactor; // База 1.1 > Потр 0.8
    m_production[static_cast<size_t>(ResourceType::Energy)]    = popFactor * 0.8 * techFactor; // База 0.8 > Потр 0.5
    m_production[static_cast<size_t>(ResourceType::Materials)] = popFactor * 0.6 * techFactor; // База 0.6 > Потр 0.3
}

void ResourceManager::updateConsumption(int population) {
    double popFactor = static_cast<double>(population) * 0.01;

    m_consumption[static_cast<size_t>(ResourceType::Food)]      = popFactor * 1.2;
    m_consumption[static_cast<size_t>(ResourceType::Money)]     = popFactor * 0.8;
    m_consumption[static_cast<size_t>(ResourceType::Energy)]    = popFactor * 0.5;
    m_consumption[static_cast<size_t>(ResourceType::Materials)] = popFactor * 0.3;
}

std::string ResourceManager::serialize() const {
    std::ostringstream oss;
    for (size_t i = 0; i < NUM_RESOURCES; ++i) {
        oss << m_resources[i] << " ";
    }
    return oss.str();
}

void ResourceManager::deserialize(const std::string& data) {
    std::istringstream iss(data);
    for (size_t i = 0; i < NUM_RESOURCES; ++i) {
        iss >> m_resources[i];
    }
}

std::string ResourceManager::getStatusString() const {
    std::ostringstream oss;
    for (size_t i = 0; i < NUM_RESOURCES; ++i) {
        auto type = static_cast<ResourceType>(i);
        std::string name = resourceTypeToString(type);
        double val = m_resources[i];
        double net = getNetIncome(type);

        std::string netStr = (net >= 0 ? "+" : "") + Utils::formatDouble(net);
        oss << "  " << Utils::padRight(name, 14) << ": "
            << Utils::padLeft(Utils::formatDouble(val, 0), 8)
            << " (" << netStr << u8"/ход)\n";
    }
    return oss.str();
}

} // namespace civ
