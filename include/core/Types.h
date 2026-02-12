#pragma once

#include <string>
#include <cstdint>

namespace civ {

// ============================================================
// Перечисления
// ============================================================

enum class Era : uint8_t {
    StoneAge = 0,
    BronzeAge,
    IronAge,
    Medieval,
    Renaissance,
    Industrial,
    Modern,
    Information,
    Space,
    COUNT
};

enum class Difficulty : uint8_t {
    Easy = 0,
    Normal,
    Hard,
    Nightmare
};

enum class EventType : uint8_t {
    Epidemic,
    EconomicCrisis,
    TechBreakthrough,
    War,
    NaturalDisaster,
    Revolution,
    GoldenAge,
    COUNT
};

enum class TechBranch : uint8_t {
    Science = 0,
    Medicine,
    Military,
    Industry,
    Space,
    COUNT
};

enum class ResourceType : uint8_t {
    Food = 0,
    Money,
    Energy,
    Materials,
    COUNT
};

enum class GameResult : uint8_t {
    InProgress,
    VictorySpace,
    VictoryEconomy,
    VictoryTech,
    DefeatPopulation,
    DefeatEcology,
    DefeatEconomy
};

// ============================================================
// Функции преобразования перечислений в строки (русский)
// ============================================================

inline std::string eraToString(Era era) {
    switch (era) {
        case Era::StoneAge:     return u8"Каменный век";
        case Era::BronzeAge:    return u8"Бронзовый век";
        case Era::IronAge:      return u8"Железный век";
        case Era::Medieval:     return u8"Средневековье";
        case Era::Renaissance:  return u8"Возрождение";
        case Era::Industrial:   return u8"Индустриальная эра";
        case Era::Modern:       return u8"Новое время";
        case Era::Information:  return u8"Информационная эра";
        case Era::Space:        return u8"Космическая эра";
        default:                return u8"Неизвестно";
    }
}

inline std::string difficultyToString(Difficulty diff) {
    switch (diff) {
        case Difficulty::Easy:      return u8"Лёгкий";
        case Difficulty::Normal:    return u8"Нормальный";
        case Difficulty::Hard:      return u8"Сложный";
        case Difficulty::Nightmare: return u8"Кошмар";
        default:                    return u8"Неизвестно";
    }
}

inline std::string eventTypeToString(EventType type) {
    switch (type) {
        case EventType::Epidemic:          return u8"Эпидемия";
        case EventType::EconomicCrisis:    return u8"Экономический кризис";
        case EventType::TechBreakthrough:  return u8"Технологический прорыв";
        case EventType::War:               return u8"Война";
        case EventType::NaturalDisaster:   return u8"Природная катастрофа";
        case EventType::Revolution:        return u8"Революция";
        case EventType::GoldenAge:         return u8"Золотой век";
        default:                           return u8"Неизвестно";
    }
}

inline std::string techBranchToString(TechBranch branch) {
    switch (branch) {
        case TechBranch::Science:   return u8"Наука";
        case TechBranch::Medicine:  return u8"Медицина";
        case TechBranch::Military:  return u8"Военное дело";
        case TechBranch::Industry:  return u8"Промышленность";
        case TechBranch::Space:     return u8"Космос";
        default:                    return u8"Неизвестно";
    }
}

inline std::string resourceTypeToString(ResourceType type) {
    switch (type) {
        case ResourceType::Food:      return u8"Еда";
        case ResourceType::Money:     return u8"Деньги";
        case ResourceType::Energy:    return u8"Энергия";
        case ResourceType::Materials: return u8"Материалы";
        default:                      return u8"Неизвестно";
    }
}

inline std::string gameResultToString(GameResult result) {
    switch (result) {
        case GameResult::InProgress:       return u8"В процессе";
        case GameResult::VictorySpace:     return u8"Победа: Освоение космоса!";
        case GameResult::VictoryEconomy:   return u8"Победа: Экономическая стабильность!";
        case GameResult::VictoryTech:      return u8"Победа: Технологическое превосходство!";
        case GameResult::DefeatPopulation: return u8"Поражение: Население вымерло";
        case GameResult::DefeatEcology:    return u8"Поражение: Экологическая катастрофа";
        case GameResult::DefeatEconomy:    return u8"Поражение: Экономический коллапс";
        default:                           return u8"Неизвестно";
    }
}

// ============================================================
// Константы
// ============================================================

constexpr int TECH_LEVEL_MAX = 100;
constexpr int VICTORY_STABLE_ECONOMY_TURNS = 50;
constexpr double ECOLOGY_COLLAPSE_THRESHOLD = 5.0;
constexpr double ECONOMY_COLLAPSE_THRESHOLD = -500.0;
constexpr int INITIAL_POPULATION = 1000;
constexpr double INITIAL_HAPPINESS = 70.0;
constexpr double INITIAL_ECOLOGY = 90.0;
constexpr double INITIAL_MILITARY = 10.0;

} // namespace civ
