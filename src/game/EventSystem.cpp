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
    // Пул событий теперь генерируется динамически в getEventsForEra
    m_eventPool.clear();
}

std::vector<GameEvent> EventSystem::getEventsForEra(Era era) const {
    std::vector<GameEvent> pool;

    // === УНИВЕРСАЛЬНЫЕ СОБЫТИЯ (Все эпохи) ===
    {
        GameEvent e;
        e.name = u8"Землетрясение";
        e.description = u8"Мощное землетрясение разрушило регион!";
        e.type = EventType::NaturalDisaster;
        e.populationMultiplier = 0.90;
        e.materialsEffect = -80.0;
        e.happinessEffect = -15.0;
        e.ecologyEffect = -5.0;
        pool.push_back(e);
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
        pool.push_back(e);
    }
    {
        GameEvent e;
        e.name = u8"Засуха";
        e.description = u8"Сильная засуха вызвала нехватку продовольствия.";
        e.type = EventType::NaturalDisaster;
        e.foodEffect = -80.0;
        e.happinessEffect = -10.0;
        e.ecologyEffect = -3.0;
        pool.push_back(e);
    }
    {
        GameEvent e;
        e.name = u8"Богатый урожай";
        e.description = (era <= Era::IronAge) ? u8"Природа была щедра к нам в этом году." : u8"Исключительный урожай наполнил амбары!";
        e.type = EventType::GoldenAge;
        e.foodEffect = 120.0;
        e.happinessEffect = 10.0;
        e.populationMultiplier = 1.03;
        pool.push_back(e);
    }

    // === РАННИЕ ЭПОХИ (Каменный, Бронзовый, Железный) ===
    if (era <= Era::IronAge) {
        {
            GameEvent e;
            e.name = u8"Набег диких зверей";
            e.description = u8"Стая хищников напала на поселение.";
            e.type = EventType::NaturalDisaster;
            e.populationMultiplier = 0.98;
            e.foodEffect = -30.0;
            e.happinessEffect = -5.0;
            pool.push_back(e);
        }
        {
            GameEvent e;
            e.name = u8"Мудрость старейшин";
            e.description = u8"Старейшины передали важные знания новому поколению.";
            e.type = EventType::TechBreakthrough;
            e.techBoost = 2;
            e.happinessEffect = 5.0;
            pool.push_back(e);
        }
        {
            GameEvent e;
            e.name = u8"Племенной конфликт";
            e.description = u8"Стычка с соседним племенем за ресурсы.";
            e.type = EventType::War;
            e.populationMultiplier = 0.95;
            e.militaryEffect = -5.0;
            e.happinessEffect = -5.0;
            e.economyEffect = -10.0;
            pool.push_back(e);
        }
    }

    // === СРЕДНИЕ ЭПОХИ (Средневековье, Возрождение) ===
    if (era >= Era::Medieval && era <= Era::Renaissance) {
        {
            GameEvent e;
            e.name = u8"Вспышка чумы";
            e.description = u8"Смертельная болезнь распространяется по городам!";
            e.type = EventType::Epidemic;
            e.populationMultiplier = 0.85;
            e.happinessEffect = -20.0;
            e.economyEffect = -50.0;
            pool.push_back(e);
        }
        {
            GameEvent e;
            e.name = u8"Рыцарский турнир";
            e.description = u8"Турнир поднял боевой дух и настроение народа.";
            e.type = EventType::GoldenAge;
            e.happinessEffect = 15.0;
            e.militaryEffect = 5.0;
            e.economyEffect = -20.0;
            pool.push_back(e);
        }
        {
            GameEvent e;
            e.name = u8"Крестьянское восстание";
            e.description = u8"Крестьяне бунтуют против высоких налогов.";
            e.type = EventType::Revolution;
            e.populationMultiplier = 0.95;
            e.happinessEffect = -15.0;
            e.economyEffect = -40.0;
            pool.push_back(e);
        }
    }

    // === ИНДУСТРИАЛЬНЫЕ И ПОЗДНИЕ ЭПОХИ (Индустриальная+) ===
    if (era >= Era::Industrial) {
        {
            GameEvent e;
            e.name = u8"Промышленный бум";
            e.description = u8"Рост производства благодаря новым фабрикам.";
            e.type = EventType::GoldenAge;
            e.economyEffect = 100.0;
            e.materialsEffect = 50.0;
            e.ecologyEffect = -10.0;
            pool.push_back(e);
        }
        {
            GameEvent e;
            e.name = u8"Забастовка рабочих";
            e.description = u8"Рабочие требуют лучших условий труда.";
            e.type = EventType::Revolution;
            e.economyEffect = -80.0;
            e.materialsEffect = -40.0;
            e.happinessEffect = -10.0;
            pool.push_back(e);
        }
        {
            GameEvent e;
            e.name = u8"Научное открытие";
            e.description = u8"Ученые совершили прорыв в лабораториях!";
            e.type = EventType::TechBreakthrough;
            e.techBoost = 4;
            e.happinessEffect = 10.0;
            pool.push_back(e);
        }
        {
            GameEvent e;
            e.name = u8"Крах фондового рынка";
            e.description = u8"Финансовый пузырь лопнул, экономика в рецессии.";
            e.type = EventType::EconomicCrisis;
            e.economyEffect = -200.0;
            e.happinessEffect = -20.0;
            pool.push_back(e);
        }
    }

    // === КОСМИЧЕСКАЯ ЭРА ===
    if (era == Era::Space) {
        {
            GameEvent e;
            e.name = u8"Контакт с внеземной жизнью";
            e.description = u8"Мы получили сигнал от другой цивилизации!";
            e.type = EventType::TechBreakthrough;
            e.techBoost = 10;
            e.happinessEffect = 25.0;
            pool.push_back(e);
        }
        {
            GameEvent e;
            e.name = u8"Авария на орбитальной станции";
            e.description = u8"Критический сбой систем жизнеобеспечения.";
            e.type = EventType::NaturalDisaster;
            e.materialsEffect = -100.0;
            e.economyEffect = -150.0;
            e.happinessEffect = -10.0;
            pool.push_back(e);
        }
        {
            GameEvent e;
            e.name = u8"Ресурсы с астероидов";
            e.description = u8"Успешная добыча редких металлов в космосе.";
            e.type = EventType::GoldenAge;
            e.materialsEffect = 200.0;
            e.economyEffect = 100.0;
            pool.push_back(e);
        }
    }

    return pool;
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
