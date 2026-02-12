#include "game/TechnologyTree.h"
#include "core/Utils.h"
#include "core/ColorOutput.h"
#include <sstream>
#include <algorithm>
#include <cmath>

namespace civ {

TechnologyTree::TechnologyTree() {
    m_branchLevels.fill(0);
    m_branchProgress.fill(0.0);
    initTechnologies();
}

void TechnologyTree::initTechnologies() {
    m_technologies = {
        // Наука
        {u8"Письменность",        TechBranch::Science,  1, 50,  u8"Позволяет вести записи"},
        {u8"Математика",          TechBranch::Science,  3, 100, u8"Основа передовой науки"},
        {u8"Физика",              TechBranch::Science,  6, 200, u8"Понимание законов природы"},
        {u8"Химия",               TechBranch::Science,  9, 350, u8"Молекулярное понимание"},
        {u8"Квантовая механика",  TechBranch::Science, 14, 600, u8"Субатомное понимание"},
        {u8"Теория термоядерного синтеза", TechBranch::Science, 18, 900, u8"Теоретическая термоядерная энергия"},

        // Медицина
        {u8"Травяная медицина",   TechBranch::Medicine,  1, 40,  u8"Базовые знания о лечении"},
        {u8"Хирургия",            TechBranch::Medicine,  4, 120, u8"Хирургические процедуры"},
        {u8"Вакцинация",          TechBranch::Medicine,  7, 250, u8"Профилактика заболеваний"},
        {u8"Антибиотики",         TechBranch::Medicine, 10, 400, u8"Лечение бактериальных инфекций"},
        {u8"Генная терапия",      TechBranch::Medicine, 15, 700, u8"Лечение генетических заболеваний"},
        {u8"Наномедицина",        TechBranch::Medicine, 19, 950, u8"Лечение на молекулярном уровне"},

        // Военное дело
        {u8"Бронзовое оружие",    TechBranch::Military,  1, 30,  u8"Базовое металлическое оружие"},
        {u8"Ковка железа",        TechBranch::Military,  3, 80,  u8"Более прочное оружие и доспехи"},
        {u8"Порох",               TechBranch::Military,  7, 200, u8"Взрывное вооружение"},
        {u8"Танки",               TechBranch::Military, 11, 450, u8"Бронированная война"},
        {u8"Ядерное оружие",      TechBranch::Military, 15, 700, u8"Оружие массового поражения"},
        {u8"Лазерная оборона",    TechBranch::Military, 19, 900, u8"Энергетические системы обороны"},

        // Промышленность
        {u8"Колесо",              TechBranch::Industry,  1, 30,  u8"Базовый транспорт"},
        {u8"Водяная мельница",    TechBranch::Industry,  3, 70,  u8"Механизмы на водной энергии"},
        {u8"Паровой двигатель",   TechBranch::Industry,  7, 200, u8"Начало промышленной революции"},
        {u8"Электричество",       TechBranch::Industry, 10, 350, u8"Электрическая сеть"},
        {u8"Автоматизация",       TechBranch::Industry, 14, 600, u8"Автоматизированное производство"},
        {u8"ИИ-производство",     TechBranch::Industry, 18, 850, u8"Полностью автономное производство"},

        // Космос
        {u8"Астрономия",          TechBranch::Space,  2, 60,  u8"Понимание космоса"},
        {u8"Ракетостроение",      TechBranch::Space,  8, 300, u8"Ракетная тяга"},
        {u8"Спутники",            TechBranch::Space, 11, 450, u8"Орбитальные технологии"},
        {u8"Космическая станция", TechBranch::Space, 14, 650, u8"Постоянная космическая среда обитания"},
        {u8"Лунная колония",      TechBranch::Space, 17, 800, u8"Колонизация Луны"},
        {u8"Межзвёздный двигатель", TechBranch::Space, 20, 1000, u8"Сверхсветовой двигатель"},
    };
}

void TechnologyTree::investInBranch(TechBranch branch, double amount) {
    size_t idx = static_cast<size_t>(branch);
    if (m_branchLevels[idx] >= MAX_BRANCH_LEVEL) return;

    m_branchProgress[idx] += amount;
    checkLevelUp(branch);
}

void TechnologyTree::checkLevelUp(TechBranch branch) {
    size_t idx = static_cast<size_t>(branch);
    while (m_branchLevels[idx] < MAX_BRANCH_LEVEL) {
        double threshold = levelUpThreshold(m_branchLevels[idx]);
        if (m_branchProgress[idx] >= threshold) {
            m_branchProgress[idx] -= threshold;
            m_branchLevels[idx]++;
        } else {
            break;
        }
    }
}

double TechnologyTree::levelUpThreshold(int currentLevel) const {
    return 50.0 + currentLevel * 30.0 + std::pow(currentLevel, 1.5) * 10.0;
}

int TechnologyTree::getBranchLevel(TechBranch branch) const {
    return m_branchLevels[static_cast<size_t>(branch)];
}

int TechnologyTree::getOverallTechLevel() const {
    int total = 0;
    for (size_t i = 0; i < NUM_BRANCHES; ++i) {
        total += m_branchLevels[i];
    }
    return total;
}

double TechnologyTree::getBranchProgress(TechBranch branch) const {
    return m_branchProgress[static_cast<size_t>(branch)];
}

double TechnologyTree::getBranchThreshold(TechBranch branch) const {
    return levelUpThreshold(m_branchLevels[static_cast<size_t>(branch)]);
}

Era TechnologyTree::getCurrentEra() const {
    int level = getOverallTechLevel();
    if (level >= 90) return Era::Space;
    if (level >= 75) return Era::Information;
    if (level >= 60) return Era::Modern;
    if (level >= 45) return Era::Industrial;
    if (level >= 32) return Era::Renaissance;
    if (level >= 20) return Era::Medieval;
    if (level >= 10) return Era::IronAge;
    if (level >= 4)  return Era::BronzeAge;
    return Era::StoneAge;
}

std::vector<const Technology*> TechnologyTree::getAvailableTechs() const {
    std::vector<const Technology*> available;
    for (const auto& tech : m_technologies) {
        if (!tech.researched) {
            int branchLvl = m_branchLevels[static_cast<size_t>(tech.branch)];
            if (branchLvl >= tech.level) {
                available.push_back(&tech);
            }
        }
    }
    return available;
}

std::vector<const Technology*> TechnologyTree::getResearchedTechs() const {
    std::vector<const Technology*> researched;
    for (const auto& tech : m_technologies) {
        if (tech.researched) {
            researched.push_back(&tech);
        }
    }
    return researched;
}

bool TechnologyTree::researchTech(const std::string& techName) {
    for (auto& tech : m_technologies) {
        if (tech.name == techName && !tech.researched) {
            int branchLvl = m_branchLevels[static_cast<size_t>(tech.branch)];
            if (branchLvl >= tech.level) {
                tech.researched = true;
                return true;
            }
        }
    }
    return false;
}

double TechnologyTree::getProductionBonus() const {
    return 1.0 + getBranchLevel(TechBranch::Industry) * 0.05;
}

double TechnologyTree::getMilitaryBonus() const {
    return 1.0 + getBranchLevel(TechBranch::Military) * 0.08;
}

double TechnologyTree::getMedicineBonus() const {
    return 1.0 + getBranchLevel(TechBranch::Medicine) * 0.04;
}

double TechnologyTree::getSpaceProgress() const {
    return static_cast<double>(getBranchLevel(TechBranch::Space)) / MAX_BRANCH_LEVEL * 100.0;
}

std::string TechnologyTree::serialize() const {
    std::ostringstream oss;
    for (size_t i = 0; i < NUM_BRANCHES; ++i) {
        oss << m_branchLevels[i] << " " << m_branchProgress[i] << " ";
    }
    oss << m_technologies.size() << " ";
    for (const auto& tech : m_technologies) {
        oss << (tech.researched ? 1 : 0) << " ";
    }
    return oss.str();
}

void TechnologyTree::deserialize(const std::string& data) {
    std::istringstream iss(data);
    for (size_t i = 0; i < NUM_BRANCHES; ++i) {
        iss >> m_branchLevels[i] >> m_branchProgress[i];
    }
    size_t techCount;
    iss >> techCount;
    for (size_t i = 0; i < std::min(techCount, m_technologies.size()); ++i) {
        int researched;
        iss >> researched;
        m_technologies[i].researched = (researched != 0);
    }
}

std::string TechnologyTree::getStatusString() const {
    std::ostringstream oss;
    oss << u8"  Эпоха: " << eraToString(getCurrentEra())
        << u8" | Общий уровень: " << getOverallTechLevel()
        << "/" << TECH_LEVEL_MAX << "\n";

    for (size_t i = 0; i < NUM_BRANCHES; ++i) {
        auto branch = static_cast<TechBranch>(i);
        std::string name = techBranchToString(branch);
        int level = m_branchLevels[i];
        double progress = m_branchProgress[i];
        double threshold = levelUpThreshold(level);

        oss << "  " << Utils::padRight(name, 16) << " Ур."
            << Utils::padLeft(std::to_string(level), 3)
            << " " << Utils::progressBar(progress, threshold, 15)
            << " " << Utils::formatDouble(progress, 0) << "/" << Utils::formatDouble(threshold, 0)
            << "\n";
    }
    return oss.str();
}

} // namespace civ
