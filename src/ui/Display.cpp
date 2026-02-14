#include "ui/Display.h"
#include "core/ColorOutput.h"
#include "core/Utils.h"
#include <iostream>
#include <iomanip>

namespace civ {

void Display::clearScreen() const {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}

void Display::waitForInput() const {
    std::cout << "\n" << ColorOutput::dim(u8"Нажмите Enter для продолжения...") << std::flush;
    std::cin.ignore(10000, '\n');
}

void Display::showSeparator(int width) const {
    std::cout << ColorOutput::dim(std::string(width, '-')) << "\n";
}

void Display::showDoubleSeparator(int width) const {
    std::cout << ColorOutput::dim(std::string(width, '=')) << "\n";
}

void Display::showTitle() const {
    clearScreen();
    std::cout << ColorOutput::cyan(
        u8"\n"
        u8"    +======================================================+\n"
        u8"    |                                                      |\n"
        u8"    |          СИМУЛЯТОР ЭВОЛЮЦИИ ЦИВИЛИЗАЦИИ              |\n"
        u8"    |                                                      |\n"
        u8"    |     От каменного века до космической эры!            |\n"
        u8"    |                                                      |\n"
        u8"    +======================================================+\n"
    ) << "\n";
}

void Display::showMainMenu() const {
    std::cout << ColorOutput::bold(u8"\n  === ГЛАВНОЕ МЕНЮ ===\n\n");
    std::cout << "  " << ColorOutput::green("[1]") << u8" Новая игра\n";
    std::cout << "  " << ColorOutput::green("[2]") << u8" Загрузить игру\n";
    std::cout << "  " << ColorOutput::green("[3]") << u8" Помощь\n";
    std::cout << "  " << ColorOutput::green("[4]") << u8" Выход\n\n";
}

void Display::showDifficultyMenu() const {
    std::cout << ColorOutput::bold(u8"\n  === ВЫБЕРИТЕ СЛОЖНОСТЬ ===\n\n");
    std::cout << "  " << ColorOutput::green("[1]") << u8" Лёгкий    - Мягкие события, медленный упадок\n";
    std::cout << "  " << ColorOutput::yellow("[2]") << u8" Нормальный - Сбалансированный опыт\n";
    std::cout << "  " << ColorOutput::red("[3]") << u8" Сложный   - Жёсткие события, быстрый упадок\n";
    std::cout << "  " << ColorOutput::magenta("[4]") << u8" Кошмар    - Экстремальный вызов\n\n";
}

void Display::showGameStatus(const Civilization& civ) const {
    showDoubleSeparator(60);
    std::cout << ColorOutput::bold(ColorOutput::cyan(
        "  " + civ.getName() + u8" | Ход: " + std::to_string(civ.getTurn()) +
        u8" | Эпоха: " + eraToString(civ.getCurrentEra())
    )) << "\n";
    showDoubleSeparator(60);

    std::cout << ColorOutput::bold(u8"\n  --- Цивилизация ---\n");
    std::cout << civ.getStatusString();

    std::cout << ColorOutput::bold(u8"\n  --- Ресурсы ---\n");
    std::cout << civ.getResources().getStatusString();

    std::cout << ColorOutput::bold(u8"\n  --- Технологии ---\n");
    std::cout << civ.getTech().getStatusString();

    showSeparator(60);
}

void Display::showResourcePanel(const Civilization& civ) const {
    std::cout << ColorOutput::bold(u8"\n  === РЕСУРСЫ ===\n\n");
    std::cout << civ.getResources().getStatusString();
}

void Display::showTechTree(const Civilization& civ) const {
    std::cout << ColorOutput::bold(u8"\n  === ДЕРЕВО ТЕХНОЛОГИЙ ===\n\n");
    std::cout << civ.getTech().getStatusString();

    auto available = civ.getTech().getAvailableTechs();
    if (!available.empty()) {
        std::cout << ColorOutput::bold(u8"\n  Доступные технологии:\n");
        for (size_t i = 0; i < available.size(); ++i) {
            std::cout << "  " << ColorOutput::green("[" + std::to_string(i + 1) + "]")
                      << " " << available[i]->name
                      << " (" << techBranchToString(available[i]->branch)
                      << u8", Цена: " << available[i]->cost << ")"
                      << " - " << ColorOutput::dim(available[i]->description) << "\n";
        }
    }

    auto researched = civ.getTech().getResearchedTechs();
    if (!researched.empty()) {
        std::cout << ColorOutput::bold(u8"\n  Исследовано:\n");
        for (const auto* tech : researched) {
            std::cout << "  " << ColorOutput::dim("[x] " + tech->name) << "\n";
        }
    }
}

void Display::showEvent(const GameEvent& event) const {
    std::cout << "\n";
    showSeparator(50);

    std::string typeColor;
    switch (event.type) {
        case EventType::Epidemic:
        case EventType::War:
        case EventType::NaturalDisaster:
            typeColor = ColorOutput::red(u8"! СОБЫТИЕ: " + event.name + " !");
            break;
        case EventType::EconomicCrisis:
        case EventType::Revolution:
            typeColor = ColorOutput::yellow(u8"! СОБЫТИЕ: " + event.name + " !");
            break;
        case EventType::TechBreakthrough:
        case EventType::GoldenAge:
            typeColor = ColorOutput::green(u8"* СОБЫТИЕ: " + event.name + " *");
            break;
        default:
            typeColor = ColorOutput::white(u8"СОБЫТИЕ: " + event.name);
    }

    std::cout << "  " << typeColor << "\n";
    std::cout << "  " << ColorOutput::dim(event.description) << "\n";

    if (event.populationMultiplier != 1.0) {
        double pct = (event.populationMultiplier - 1.0) * 100.0;
        std::string effect = Utils::formatDouble(pct) + u8"% население";
        std::cout << "  " << (pct < 0 ? ColorOutput::red(effect) : ColorOutput::green("+" + effect)) << "\n";
    }
    if (event.happinessEffect != 0) {
        std::string effect = Utils::formatDouble(event.happinessEffect) + u8" счастье";
        std::cout << "  " << (event.happinessEffect < 0 ? ColorOutput::red(effect) : ColorOutput::green("+" + effect)) << "\n";
    }
    if (event.economyEffect != 0) {
        std::string effect = Utils::formatDouble(event.economyEffect) + u8" деньги";
        std::cout << "  " << (event.economyEffect < 0 ? ColorOutput::red(effect) : ColorOutput::green("+" + effect)) << "\n";
    }
    if (event.ecologyEffect != 0) {
        std::string effect = Utils::formatDouble(event.ecologyEffect) + u8" экология";
        std::cout << "  " << (event.ecologyEffect < 0 ? ColorOutput::red(effect) : ColorOutput::green("+" + effect)) << "\n";
    }
    if (event.techBoost > 0) {
        std::cout << "  " << ColorOutput::green("+" + std::to_string(event.techBoost) + u8" технологии") << "\n";
    }

    showSeparator(50);
}

void Display::showTurnMenu(const Civilization& /*civ*/) const {
    std::cout << ColorOutput::bold(u8"\n  === ДЕЙСТВИЯ ===\n\n");
    std::cout << "  " << ColorOutput::green("[1]") << u8" Следующий ход\n";
    std::cout << "  " << ColorOutput::green("[2]") << u8" Инвестировать ресурсы\n";
    std::cout << "  " << ColorOutput::green("[3]") << u8" Исследовать технологию\n";
    std::cout << "  " << ColorOutput::green("[4]") << u8" Полный статус\n";
    std::cout << "  " << ColorOutput::green("[5]") << u8" Дерево технологий\n";
    std::cout << "  " << ColorOutput::green("[6]") << u8" Журнал событий\n";
    std::cout << "  " << ColorOutput::green("[7]") << u8" Сохранить игру\n";
    std::cout << "  " << ColorOutput::green("[8]") << u8" Помощь\n";
    std::cout << "  " << ColorOutput::red("[9]") << u8" Выйти в меню\n\n";
}

void Display::showInvestmentMenu(const Civilization& civ) const {
    std::cout << ColorOutput::bold(u8"\n  === ИНВЕСТИЦИИ В ТЕХНОЛОГИИ ===\n\n");
    double money = civ.getResources().getResource(ResourceType::Money);
    std::cout << u8"  Доступно денег: " << ColorOutput::yellow(Utils::formatDouble(money, 0)) << "\n\n";

    for (int i = 0; i < static_cast<int>(TechBranch::COUNT); ++i) {
        auto branch = static_cast<TechBranch>(i);
        std::string name = techBranchToString(branch);
        int level = civ.getTech().getBranchLevel(branch);
        std::cout << "  " << ColorOutput::green("[" + std::to_string(i + 1) + "]")
                  << " " << Utils::padRight(name, 16)
                  << " (Ур." << level << ")\n";
    }
    std::cout << "  " << ColorOutput::red("[0]") << u8" Отмена\n\n";
}

void Display::showEventLog(const EventSystem& events) const {
    std::cout << ColorOutput::bold(u8"\n  === ЖУРНАЛ СОБЫТИЙ ===\n\n");
    const auto& history = events.getEventHistory();
    if (history.empty()) {
        std::cout << u8"  Событий пока не было.\n";
        return;
    }

    size_t start = (history.size() > 20) ? history.size() - 20 : 0;
    for (size_t i = start; i < history.size(); ++i) {
        const auto& e = history[i];
        std::string typeStr = eventTypeToString(e.type);
        std::cout << "  [" << Utils::padLeft(std::to_string(i + 1), 3) << "] "
                  << Utils::padRight(typeStr, 24) << " - " << e.name << "\n";
    }
}

void Display::showVictory(GameResult result) const {
    clearScreen();
    std::cout << ColorOutput::green(
        u8"\n"
        u8"    +==========================================+\n"
        u8"    |                                          |\n"
        u8"    |            П О Б Е Д А !                |\n"
        u8"    |                                          |\n"
        u8"    +==========================================+\n"
    ) << "\n";

    std::cout << "  " << ColorOutput::bold(ColorOutput::green(gameResultToString(result))) << "\n\n";
    std::cout << u8"  Ваша цивилизация достигла величия!\n";
}

void Display::showDefeat(GameResult result) const {
    clearScreen();
    std::cout << ColorOutput::red(
        u8"\n"
        u8"    +==========================================+\n"
        u8"    |                                          |\n"
        u8"    |          П О Р А Ж Е Н И Е             |\n"
        u8"    |                                          |\n"
        u8"    +==========================================+\n"
    ) << "\n";

    std::cout << "  " << ColorOutput::bold(ColorOutput::red(gameResultToString(result))) << "\n\n";
    std::cout << u8"  Ваша цивилизация пала...\n";
}

void Display::showHelp() const {
    clearScreen();
    std::cout << ColorOutput::bold(ColorOutput::cyan(u8"\n  === ПОМОЩЬ ===\n\n"));

    std::cout << ColorOutput::bold(u8"  ЦЕЛЬ:\n");
    std::cout << u8"  Проведите цивилизацию от Каменного века до Космической эры!\n\n";

    std::cout << ColorOutput::bold(u8"  УСЛОВИЯ ПОБЕДЫ:\n");
    std::cout << "  " << ColorOutput::green("*") << u8" Космическая эра     - Достигнуть эпохи освоения космоса\n\n";

    std::cout << ColorOutput::bold(u8"  УСЛОВИЯ ПОРАЖЕНИЯ:\n");
    std::cout << "  " << ColorOutput::red("x") << u8" Население достигло 0\n";
    std::cout << "  " << ColorOutput::red("x") << u8" Экология рухнула (ниже 5%)\n";
    std::cout << "  " << ColorOutput::red("x") << u8" Экономический коллапс (банкротство + нет еды)\n\n";

    std::cout << ColorOutput::bold(u8"  СОВЕТЫ:\n");
    std::cout << u8"  - Балансируйте инвестиции между ветками технологий\n";
    std::cout << u8"  - Наука помогает экологии, Промышленность вредит\n";
    std::cout << u8"  - Медицина улучшает рост населения\n";
    std::cout << u8"  - Поддерживайте высокое счастье для лучшего роста\n";
    std::cout << u8"  - Случайные события могут помочь или навредить!\n\n";
}

void Display::showEraArt(Era era) const {
    switch (era) {
        case Era::StoneAge:
            std::cout << ColorOutput::dim(
                u8"\n      /\\      Каменный век\n"
                u8"     /  \\     Примитивные орудия\n"
                u8"    /    \\    и огонь\n"
                u8"   /______\\\n"
            ) << "\n";
            break;
        case Era::BronzeAge:
            std::cout << ColorOutput::yellow(
                u8"\n    _/|\\_     Бронзовый век\n"
                u8"   / _|_ \\   Начало обработки\n"
                u8"  |_/ | \\_|  металлов\n"
                u8"     |__|\n"
            ) << "\n";
            break;
        case Era::Medieval:
            std::cout << ColorOutput::white(
                u8"\n    |T|T|     Средневековье\n"
                u8"    |=|=|     Замки и\n"
                u8"   /|_|_|\\   королевства\n"
                u8"  |_______|\n"
            ) << "\n";
            break;
        case Era::Industrial:
            std::cout << ColorOutput::dim(
                u8"\n    ___|___   Индустриальная эра\n"
                u8"   |  |||  |  Пар и\n"
                u8"   |  |||  |  фабрики\n"
                u8"   |__|||__|\n"
            ) << "\n";
            break;
        case Era::Space:
            std::cout << ColorOutput::cyan(
                u8"\n      /\\      Космическая эра\n"
                u8"     /  \\     Звёзды\n"
                u8"    | ** |    ждут!\n"
                u8"    |    |\n"
                u8"   /|    |\\\n"
                u8"  /_|____|_\\\n"
            ) << "\n";
            break;
        default:
            break;
    }
}

void Display::showBar(const std::string& label, double value, double maxVal,
                      int width, const std::string& /*color*/) const {
    std::string bar = Utils::progressBar(value, maxVal, width);
    std::cout << "  " << Utils::padRight(label, 14) << ": " << bar
              << " " << Utils::formatDouble(value) << "/" << Utils::formatDouble(maxVal) << "\n";
}

} // namespace civ
