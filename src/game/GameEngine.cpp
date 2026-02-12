#include "game/GameEngine.h"
#include "core/Logger.h"
#include "core/ColorOutput.h"
#include "core/Utils.h"
#include <iostream>
#include <stdexcept>

namespace civ {

GameEngine::GameEngine() = default;
GameEngine::~GameEngine() { cleanup(); }

void GameEngine::initSystems() {
    m_civ = std::make_unique<Civilization>();
    m_events = std::make_unique<EventSystem>();
    m_saveSystem = std::make_unique<SaveSystem>();
    m_display = std::make_unique<Display>();
}

void GameEngine::cleanup() {
    m_civ.reset();
    m_events.reset();
    m_saveSystem.reset();
    m_display.reset();
}

void GameEngine::run() {
    try {
        ColorOutput::init();
        Logger::instance().init("civsim.log", LogLevel::Debug);
        Logger::instance().info("=== Civilization Simulator Started ===");

        initSystems();
        m_running = true;

        while (m_running) {
            showMainMenu();
        }

        Logger::instance().info("=== Civilization Simulator Ended ===");
        Logger::instance().shutdown();
    }
    catch (const std::exception& e) {
        Logger::instance().critical(std::string("Fatal error: ") + e.what());
        std::cerr << ColorOutput::error(u8"Критическая ошибка: " + std::string(e.what())) << std::endl;
    }
}

void GameEngine::showMainMenu() {
    m_display->showTitle();
    m_display->showMainMenu();

    int choice = InputHandler::getInt(u8"Выбор", 1, 4);

    switch (choice) {
        case 1:
            startNewGame();
            break;
        case 2:
            loadGame();
            break;
        case 3:
            m_display->showHelp();
            InputHandler::waitForKey();
            break;
        case 4:
            m_running = false;
            m_display->clearScreen();
            std::cout << ColorOutput::cyan(u8"\n  Спасибо за игру в Симулятор Цивилизации!\n\n");
            break;
    }
}

void GameEngine::startNewGame() {
    m_display->clearScreen();
    m_display->showDifficultyMenu();

    int diffChoice = InputHandler::getInt(u8"Выберите сложность", 1, 4);
    m_difficulty = static_cast<Difficulty>(diffChoice - 1);

    std::string name = InputHandler::getString(u8"Введите название цивилизации");
    if (name.empty()) name = u8"Человечество";

    m_civ = std::make_unique<Civilization>(name);
    m_events = std::make_unique<EventSystem>();
    m_events->init(m_difficulty);
    m_result = GameResult::InProgress;

    Logger::instance().info("New game started: " + name +
                           " (Difficulty: " + difficultyToString(m_difficulty) + ")");

    m_display->clearScreen();
    std::cout << ColorOutput::bold(ColorOutput::cyan(
        u8"\n  === Ваша цивилизация \"" + name + u8"\" начинает свой путь! ===\n"
    ));
    std::cout << u8"  Сложность: " << ColorOutput::yellow(difficultyToString(m_difficulty)) << "\n";
    m_display->showEraArt(Era::StoneAge);
    InputHandler::waitForKey();

    gameLoop();
}

void GameEngine::loadGame() {
    if (!SaveSystem::saveExists()) {
        std::cout << "\n  " << ColorOutput::warning(u8"Файл сохранения не найден.") << "\n";
        InputHandler::waitForKey();
        return;
    }

    m_civ = std::make_unique<Civilization>();
    m_events = std::make_unique<EventSystem>();

    if (m_saveSystem->loadGame(*m_civ, *m_events, m_difficulty)) {
        m_result = GameResult::InProgress;
        std::cout << "\n  " << ColorOutput::success(u8"Игра успешно загружена!") << "\n";
        Logger::instance().info("Game loaded from save file");
        InputHandler::waitForKey();
        gameLoop();
    } else {
        std::cout << "\n  " << ColorOutput::error(u8"Ошибка загрузки: " + m_saveSystem->getLastError()) << "\n";
        InputHandler::waitForKey();
    }
}

void GameEngine::gameLoop() {
    static constexpr auto QUIT_SENTINEL = static_cast<GameResult>(255);
    Era previousEra = m_civ->getCurrentEra();

    while (m_result == GameResult::InProgress) {
        m_display->clearScreen();
        m_display->showGameStatus(*m_civ);

        Era currentEra = m_civ->getCurrentEra();
        if (currentEra != previousEra) {
            std::cout << "\n  " << ColorOutput::bold(ColorOutput::magenta(
                u8"*** СМЕНА ЭПОХИ: " + eraToString(currentEra) + " ***"
            )) << "\n";
            m_display->showEraArt(currentEra);
            Logger::instance().info("Era changed to: " + eraToString(currentEra));
            previousEra = currentEra;
        }

        handlePlayerAction();

        if (m_result == QUIT_SENTINEL) {
            m_result = GameResult::InProgress;
            return;
        }

        checkEndConditions();
    }

    showEndScreen();
}

void GameEngine::handlePlayerAction() {
    m_display->showTurnMenu(*m_civ);

    int choice = InputHandler::getInt(u8"Действие", 1, 9);

    switch (choice) {
        case 1:
            processTurn();
            break;
        case 2:
            handleInvestment();
            break;
        case 3:
            handleTechResearch();
            break;
        case 4:
            m_display->clearScreen();
            m_display->showGameStatus(*m_civ);
            InputHandler::waitForKey();
            break;
        case 5:
            m_display->clearScreen();
            m_display->showTechTree(*m_civ);
            InputHandler::waitForKey();
            break;
        case 6:
            m_display->clearScreen();
            m_display->showEventLog(*m_events);
            InputHandler::waitForKey();
            break;
        case 7:
            handleSaveGame();
            break;
        case 8:
            m_display->showHelp();
            InputHandler::waitForKey();
            break;
        case 9:
            if (InputHandler::getYesNo(u8"Сохранить перед выходом?")) {
                handleSaveGame();
            }
            m_result = static_cast<GameResult>(255);
            return;
    }
}

void GameEngine::processTurn() {
    Logger::instance().info("=== Turn " + std::to_string(m_civ->getTurn() + 1) + " ===");

    GameEvent event = m_events->generateEvent(m_civ->getCurrentEra(), m_civ->getTurn());
    m_events->recordEvent(event);

    m_display->showEvent(event);

    m_civ->applyEvent(event);
    m_civ->processTurn();

    Logger::instance().info("Turn processed. Pop: " + std::to_string(m_civ->getPopulation()) +
                           " Tech: " + std::to_string(m_civ->getTech().getOverallTechLevel()));

    InputHandler::waitForKey();
}

void GameEngine::handleInvestment() {
    m_display->clearScreen();
    m_display->showInvestmentMenu(*m_civ);

    int choice = InputHandler::getInt(u8"Выберите ветку (0 - отмена)", 0,
                                       static_cast<int>(TechBranch::COUNT));
    if (choice == 0) return;

    auto branch = static_cast<TechBranch>(choice - 1);
    double available = m_civ->getResources().getResource(ResourceType::Money);

    if (available <= 0) {
        std::cout << "  " << ColorOutput::error(u8"Нет денег для инвестиций!") << "\n";
        InputHandler::waitForKey();
        return;
    }

    double maxInvest = std::min(available, 500.0);
    double amount = InputHandler::getDouble(u8"Сумма инвестиций (макс " +
                                            Utils::formatDouble(maxInvest, 0) + ")", 0, maxInvest);

    if (amount > 0) {
        m_civ->getResources().removeResource(ResourceType::Money, amount);
        m_civ->getTech().investInBranch(branch, amount);

        std::cout << "  " << ColorOutput::success(u8"Инвестировано " + Utils::formatDouble(amount, 0) +
                  u8" в " + techBranchToString(branch) + "!") << "\n";
        Logger::instance().info("Invested " + Utils::formatDouble(amount) +
                               " in " + techBranchToString(branch));
    }

    InputHandler::waitForKey();
}

void GameEngine::handleTechResearch() {
    m_display->clearScreen();
    m_display->showTechTree(*m_civ);

    auto available = m_civ->getTech().getAvailableTechs();
    if (available.empty()) {
        std::cout << "\n  " << ColorOutput::warning(u8"Нет доступных технологий для исследования.") << "\n";
        std::cout << "  " << ColorOutput::dim(u8"Инвестируйте в ветки технологий для открытия новых.") << "\n";
        InputHandler::waitForKey();
        return;
    }

    std::cout << "\n  " << ColorOutput::cyan(u8"Выберите технологию (0 - отмена)") << "\n";
    int choice = InputHandler::getInt(u8"Выбор", 0, static_cast<int>(available.size()));

    if (choice == 0) return;

    const Technology* tech = available[choice - 1];
    double money = m_civ->getResources().getResource(ResourceType::Money);

    if (money < tech->cost) {
        std::cout << "  " << ColorOutput::error(u8"Недостаточно денег! Нужно " +
                  std::to_string(tech->cost) + u8", есть " + Utils::formatDouble(money, 0)) << "\n";
        InputHandler::waitForKey();
        return;
    }

    m_civ->getResources().removeResource(ResourceType::Money, tech->cost);
    if (m_civ->getTech().researchTech(tech->name)) {
        std::cout << "  " << ColorOutput::success(u8"Исследовано: " + tech->name + "!") << "\n";
        std::cout << "  " << ColorOutput::dim(tech->description) << "\n";
        Logger::instance().info("Researched technology: " + tech->name);
    }

    InputHandler::waitForKey();
}

void GameEngine::handleSaveGame() {
    if (m_saveSystem->saveGame(*m_civ, *m_events, m_difficulty)) {
        std::cout << "\n  " << ColorOutput::success(u8"Игра успешно сохранена!") << "\n";
    } else {
        std::cout << "\n  " << ColorOutput::error(u8"Ошибка сохранения: " + m_saveSystem->getLastError()) << "\n";
    }
    InputHandler::waitForKey();
}

void GameEngine::checkEndConditions() {
    m_result = m_civ->checkGameResult();
}

void GameEngine::showEndScreen() {
    if (m_result == GameResult::InProgress) return;

    switch (m_result) {
        case GameResult::VictorySpace:
        case GameResult::VictoryEconomy:
        case GameResult::VictoryTech:
            m_display->showVictory(m_result);
            break;
        case GameResult::DefeatPopulation:
        case GameResult::DefeatEcology:
        case GameResult::DefeatEconomy:
            m_display->showDefeat(m_result);
            break;
        default:
            break;
    }

    std::cout << u8"\n  Ходов сыграно: " << m_civ->getTurn() << "\n";
    std::cout << u8"  Финальное население: " << Utils::formatNumber(m_civ->getPopulation()) << "\n";
    std::cout << u8"  Уровень технологий: " << m_civ->getTech().getOverallTechLevel() << "\n";
    std::cout << u8"  Финальная эпоха: " << eraToString(m_civ->getCurrentEra()) << "\n";

    Logger::instance().info("Game ended: " + gameResultToString(m_result) +
                           " at turn " + std::to_string(m_civ->getTurn()));

    InputHandler::waitForKey();

    m_result = GameResult::InProgress;
}

} // namespace civ
