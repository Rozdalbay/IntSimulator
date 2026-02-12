#include "game/EventSystem.h"
#include "core/Utils.h"
#include "core/Logger.h"
#include <sstream>
#include <algorithm>

namespace civ {

EventSystem::EventSystem() {
    buildEventPool();
}

void EventSystem::init(Difficulty difficulty) {
    m_difficulty = difficulty;
    m_eventHistory.clear();
    buildEventPool();
}

void EventSystem::setDifficulty(Difficulty difficulty) {
    m_difficulty = difficulty;
}

double EventSystem::getDifficultyMultiplier() const {
    switch (m_difficulty) {
        case Difficulty::Easy:      return 0.6;
        case Difficulty::Normal:    return 1.0;
        case Difficulty::Hard:      return 1.4;
        case Difficulty::Nightmare: return 2.0;
        default:                    return 1.0;
    }
}

void EventSystem::buildEventPool() {
    m_eventPool.clear();

    // === ЭПИДЕМИИ ===
    {
        GameEvent e;
        e.name = u8"Вспышка чумы";
        e.description = u8"Смертельная чума охватила население!";
        e.type = EventType::Epidemic;
        e.populationMultiplier = 0.85;
        e.happinessEffect = -15.0;
        e.economyEffect = -50.0;
        m_eventPool.push_back(e);
    }
    {
        GameEvent e;
        e.name = u8"Сезонная болезнь";
        e.description = u8"Сезонная болезнь снижает производительность.";
        e.type = EventType::Epidemic;
        e.populationMultiplier = 0.97;
        e.happinessEffect = -5.0;
        e.foodEffect = -20.0;
        m_eventPool.push_back(e);
    }

    // === ЭКОНОМИЧЕСКИЙ КРИЗИС ===
    {
        GameEvent e;
        e.name = u8"Крах рынка";
        e.description = u8"Экономика рухнула! Рынки в хаосе.";
        e.type = EventType::EconomicCrisis;
        e.economyEffect = -200.0;
        e.happinessEffect = -20.0;
        m_eventPool.push_back(e);
    }
    {
        GameEvent e;
        e.name = u8"Нарушение торговли";
        e.description = u8"Торговые пути нарушены, возникает дефицит.";
        e.type = EventType::EconomicCrisis;
        e.economyEffect = -80.0;
        e.foodEffect = -30.0;
        e.materialsEffect = -20.0;
        m_eventPool.push_back(e);
    }

    // === ТЕХНОЛОГИЧЕСКИЙ ПРОРЫВ ===
    {
        GameEvent e;
        e.name = u8"Научное открытие";
        e.description = u8"Гениальный учёный совершил прорывное открытие!";
        e.type = EventType::TechBreakthrough;
        e.techBoost = 3;
        e.happinessEffect = 10.0;
        m_eventPool.push_back(e);
    }
    {
        GameEvent e;
        e.name = u8"Волна инноваций";
        e.description = u8"Волна инноваций охватила цивилизацию!";
        e.type = EventType::TechBreakthrough;
        e.techBoost = 2;
        e.economyEffect = 50.0;
        e.materialsEffect = 30.0;
        m_eventPool.push_back(e);
    }

    // === ВОЙНА ===
    {
        GameEvent e;
        e.name = u8"Пограничный конфликт";
        e.description = u8"Соседнее племя атакует наши границы!";
        e.type = EventType::War;
        e.populationMultiplier = 0.92;
        e.militaryEffect = -10.0;
        e.happinessEffect = -10.0;
        e.economyEffect = -60.0;
        m_eventPool.push_back(e);
    }
    {
        GameEvent e;
        e.name = u8"Полномасштабная война";
        e.description = u8"Началась война! Ресурсы направлены на армию.";
        e.type = EventType::War;
        e.populationMultiplier = 0.80;
        e.militaryEffect = 15.0;
        e.happinessEffect = -25.0;
        e.economyEffect = -150.0;
        e.materialsEffect = -50.0;
        m_eventPool.push_back(e);
    }

    // === ПРИРОДНАЯ КАТАСТРОФА ===
    {
        GameEvent e;
        e.name = u8"Землетрясение";
        e.description = u8"Мощное землетрясение разрушило регион!";
        e.type = EventType::NaturalDisaster;
        e.populationMultiplier = 0.90;
        e.materialsEffect = -80.0;
        e.happinessEffect = -15.0;
        e.ecologyEffect = -5.0;
        m_eventPool.push_back(e);
    }
    {
        GameEvent e;
        e.name = u8"Великое наводнение";
        e.description = u8"Наводнение уничтожило поля и поселения.";
        e.type = EventType::NaturalDisaster;
        e.foodEffect = -100.0;
        e.populationMultiplier = 0.93;
        e.ecologyEffect = -8.0;
        e.happinessEffect = -12.0;
        m_eventPool.push_back(e);
    }
    {
        GameEvent e;
        e.name = u8"Засуха";
        e.description = u8"Сильная засуха вызвала нехватку продовольствия.";
        e.type = EventType::NaturalDisaster;
        e.foodEffect = -80.0;
        e.happinessEffect = -10.0;
        e.ecologyEffect = -3.0;
        m_eventPool.push_back(e);
    }

    // === РЕВОЛЮЦИЯ ===
    {
        GameEvent e;
        e.name = u8"Народное восстание";
        e.description = u8"Народ требует перемен! Началась революция.";
        e.type = EventType::Revolution;
        e.populationMultiplier = 0.95;
        e.happinessEffect = 15.0;
        e.militaryEffect = -15.0;
        e.economyEffect = -100.0;
        m_eventPool.push_back(e);
    }
    {
        GameEvent e;
        e.name = u8"Реформаторское движение";
        e.description = u8"Мирное реформаторское движение улучшило управление.";
        e.type = EventType::Revolution;
        e.happinessEffect = 20.0;
        e.economyEffect = 30.0;
        m_eventPool.push_back(e);
    }

    // === ЗОЛОТОЙ ВЕК ===
    {
        GameEvent e;
        e.name = u8"Золотой век";
        e.description = u8"Процветание! Цивилизация вступила в золотой век.";
        e.type = EventType::GoldenAge;
        e.happinessEffect = 25.0;
        e.economyEffect = 150.0;
        e.foodEffect = 50.0;
        e.energyEffect = 30.0;
        e.populationMultiplier = 1.05;
        m_eventPool.push_back(e);
    }
    {
        GameEvent e;
        e.name = u8"Культурный ренессанс";
        e.description = u8"Искусство и культура расцветают, вдохновляя народ!";
        e.type = EventType::GoldenAge;
        e.happinessEffect = 20.0;
        e.techBoost = 1;
        e.economyEffect = 80.0;
        m_eventPool.push_back(e);
    }
    {
        GameEvent e;
        e.name = u8"Богатый урожай";
        e.description = u8"Исключительный урожай наполнил амбары!";
        e.type = EventType::GoldenAge;
        e.foodEffect = 120.0;
        e.happinessEffect = 10.0;
        e.populationMultiplier = 1.03;
        m_eventPool.push_back(e);
    }
}

std::vector<GameEvent> EventSystem::getEventsForEra(Era /*era*/) const {
    return m_eventPool;
}

GameEvent EventSystem::generateEvent(Era currentEra, int /*turn*/) const {
    auto pool = getEventsForEra(currentEra);
    if (pool.empty()) {
        GameEvent fallback;
        fallback.name = u8"Тихий год";
        fallback.description = u8"Ничего значительного не произошло.";
        fallback.type = EventType::GoldenAge;
        return fallback;
    }

    double diffMult = getDifficultyMultiplier();

    // 30% шанс мирного года
    double noEventChance = 0.30 / diffMult;
    if (Utils::randomChance(noEventChance)) {
        GameEvent quiet;
        quiet.name = u8"Мирный год";
        quiet.description = u8"Спокойный и безмятежный год прошёл.";
        quiet.type = EventType::GoldenAge;
        quiet.happinessEffect = 2.0;
        quiet.foodEffect = 10.0;
        return quiet;
    }

    // Выбор случайного события
    int idx = Utils::randomInt(0, static_cast<int>(pool.size()) - 1);
    GameEvent event = pool[idx];

    // Масштабирование негативных эффектов по сложности
    if (event.populationMultiplier < 1.0) {
        double loss = 1.0 - event.populationMultiplier;
        event.populationMultiplier = 1.0 - loss * diffMult;
    }
    if (event.happinessEffect < 0) {
        event.happinessEffect *= diffMult;
    }
    if (event.economyEffect < 0) {
        event.economyEffect *= diffMult;
    }
    if (event.ecologyEffect < 0) {
        event.ecologyEffect *= diffMult;
    }

    // Масштабирование позитивных эффектов обратно пропорционально
    if (event.happinessEffect > 0) {
        event.happinessEffect /= diffMult;
    }
    if (event.economyEffect > 0) {
        event.economyEffect /= diffMult;
    }

    return event;
}

void EventSystem::recordEvent(const GameEvent& event) {
    m_eventHistory.push_back(event);
}

const std::vector<GameEvent>& EventSystem::getEventHistory() const {
    return m_eventHistory;
}

std::string EventSystem::serialize() const {
    std::ostringstream oss;
    oss << static_cast<int>(m_difficulty) << " ";
    oss << m_eventHistory.size() << " ";
    for (const auto& e : m_eventHistory) {
        oss << e.name.size() << " " << e.name << " "
            << static_cast<int>(e.type) << " ";
    }
    return oss.str();
}

void EventSystem::deserialize(const std::string& data) {
    std::istringstream iss(data);
    int diff;
    iss >> diff;
    m_difficulty = static_cast<Difficulty>(diff);

    size_t histSize;
    iss >> histSize;
    m_eventHistory.clear();
    for (size_t i = 0; i < histSize; ++i) {
        GameEvent e;
        size_t nameLen;
        iss >> nameLen;
        iss.ignore(1);
        e.name.resize(nameLen);
        iss.read(&e.name[0], nameLen);
        int type;
        iss >> type;
        e.type = static_cast<EventType>(type);
        m_eventHistory.push_back(e);
    }
}

} // namespace civ
