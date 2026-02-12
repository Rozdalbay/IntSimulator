#include "ui/Win32Gui.h"
#include "core/Utils.h"
#include <Windows.h>
#include <commctrl.h>
#include <string>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "comctl32.lib")

namespace civ {

Win32Gui::Win32Gui()
    : m_hInstance(nullptr)
    , m_mainWindow(nullptr)
    , m_running(false)
    , m_difficulty(Difficulty::Normal)
{
}

Win32Gui::~Win32Gui() {
    if (m_mainWindow) {
        DestroyWindow(m_mainWindow);
    }
    UnregisterClassW(CLASS_NAME, m_hInstance);
}

std::wstring Win32Gui::toWStr(const std::string& str) {
    if (str.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstr(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size);
    return wstr;
}

int Win32Gui::run(HINSTANCE hInstance, int nCmdShow) {
    m_hInstance = hInstance;

    INITCOMMONCONTROLSEX icex;
    icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icex);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = StaticWndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = CLASS_NAME;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

    if (!RegisterClassExW(&wc)) {
        MessageBoxW(nullptr, L"Ошибка регистрации окна", L"Ошибка", MB_OK | MB_ICONERROR);
        return 1;
    }

    m_mainWindow = CreateWindowExW(
        0, CLASS_NAME, WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr, nullptr, hInstance, this
    );

    if (!m_mainWindow) {
        MessageBoxW(nullptr, L"Ошибка создания окна", L"Ошибка", MB_OK | MB_ICONERROR);
        return 1;
    }

    createControls();
    ShowWindow(m_mainWindow, nCmdShow);
    UpdateWindow(m_mainWindow);

    MSG msg = {};
    m_running = true;
    while (m_running && GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

void Win32Gui::createControls() {
    HFONT hFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                                CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                VARIABLE_PITCH, L"Segoe UI");
    HFONT hBoldFont = CreateFontW(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                                   CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                   VARIABLE_PITCH, L"Segoe UI");
    HFONT hTitleFont = CreateFontW(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                    DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                                    CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                    VARIABLE_PITCH, L"Segoe UI");
    HFONT hSmallFont = CreateFontW(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                    DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                                    CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                    VARIABLE_PITCH, L"Segoe UI");

    int x = 20, y = 20;

    // Title
    HWND hTitle = CreateWindowW(L"STATIC", L"СИМУЛЯТОР ЭВОЛЮЦИИ ЦИВИЛИЗАЦИИ",
                                 WS_VISIBLE | SS_LEFT, x, y, 500, 40,
                                 m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hTitle, WM_SETFONT, (WPARAM)hTitleFont, 0);
    y += 50;

    // Era and Turn info
    m_eraLabel = CreateWindowW(L"STATIC", L"Эпоха: Каменный век",
                                WS_VISIBLE | SS_LEFT, x, y, 300, 25,
                                m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_eraLabel, WM_SETFONT, (WPARAM)hBoldFont, 0);
    y += 30;

    m_turnLabel = CreateWindowW(L"STATIC", L"Ход: 0",
                                WS_VISIBLE | SS_LEFT, x, y, 200, 20,
                                m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_turnLabel, WM_SETFONT, (WPARAM)hFont, 0);
    y += 35;

    // ===== LEFT PANEL - Stats =====
    int panelX = x;
    
    // Population
    HWND hPopTitle = CreateWindowW(L"STATIC", L"Население",
                                     WS_VISIBLE | SS_LEFT, panelX, y, 150, 20,
                                     m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hPopTitle, WM_SETFONT, (WPARAM)hFont, 0);
    y += 20;
    
    m_populationLabel = CreateWindowW(L"STATIC", L"1 000",
                                       WS_VISIBLE | SS_LEFT, panelX, y, 150, 20,
                                       m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_populationLabel, WM_SETFONT, (WPARAM)hSmallFont, 0);
    y += 20;
    
    m_populationBar = CreateWindowW(PROGRESS_CLASSW, nullptr,
                                     WS_VISIBLE | PBS_SMOOTH, panelX, y, 250, 18,
                                     m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_populationBar, PBM_SETRANGE, 0, MAKELPARAM(0, 10000));
    SendMessageW(m_populationBar, PBM_SETPOS, 1000, 0);
    y += 30;

    // Happiness
    HWND hHappyTitle = CreateWindowW(L"STATIC", L"Счастье",
                                      WS_VISIBLE | SS_LEFT, panelX, y, 150, 20,
                                      m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hHappyTitle, WM_SETFONT, (WPARAM)hFont, 0);
    y += 20;
    
    m_happinessLabel = CreateWindowW(L"STATIC", L"70%",
                                      WS_VISIBLE | SS_LEFT, panelX, y, 150, 20,
                                      m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_happinessLabel, WM_SETFONT, (WPARAM)hSmallFont, 0);
    y += 20;
    
    m_happinessBar = CreateWindowW(PROGRESS_CLASSW, nullptr,
                                    WS_VISIBLE | PBS_SMOOTH, panelX, y, 250, 18,
                                    m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_happinessBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    SendMessageW(m_happinessBar, PBM_SETPOS, 70, 0);
    y += 30;

    // Ecology
    HWND hEcoTitle = CreateWindowW(L"STATIC", L"Экология",
                                    WS_VISIBLE | SS_LEFT, panelX, y, 150, 20,
                                    m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hEcoTitle, WM_SETFONT, (WPARAM)hFont, 0);
    y += 20;
    
    m_ecologyLabel = CreateWindowW(L"STATIC", L"90%",
                                    WS_VISIBLE | SS_LEFT, panelX, y, 150, 20,
                                    m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_ecologyLabel, WM_SETFONT, (WPARAM)hSmallFont, 0);
    y += 20;
    
    m_ecologyBar = CreateWindowW(PROGRESS_CLASSW, nullptr,
                                  WS_VISIBLE | PBS_SMOOTH, panelX, y, 250, 18,
                                  m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_ecologyBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    SendMessageW(m_ecologyBar, PBM_SETPOS, 90, 0);
    y += 30;

    // Military
    HWND hMilTitle = CreateWindowW(L"STATIC", L"Армия",
                                    WS_VISIBLE | SS_LEFT, panelX, y, 150, 20,
                                    m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hMilTitle, WM_SETFONT, (WPARAM)hFont, 0);
    y += 20;
    
    m_militaryLabel = CreateWindowW(L"STATIC", L"10",
                                     WS_VISIBLE | SS_LEFT, panelX, y, 150, 20,
                                     m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_militaryLabel, WM_SETFONT, (WPARAM)hSmallFont, 0);
    y += 20;
    
    m_militaryBar = CreateWindowW(PROGRESS_CLASSW, nullptr,
                                   WS_VISIBLE | PBS_SMOOTH, panelX, y, 250, 18,
                                   m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_militaryBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    SendMessageW(m_militaryBar, PBM_SETPOS, 10, 0);
    y += 30;

    // Economy
    HWND hEconTitle = CreateWindowW(L"STATIC", L"Экономика",
                                     WS_VISIBLE | SS_LEFT, panelX, y, 150, 20,
                                     m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hEconTitle, WM_SETFONT, (WPARAM)hFont, 0);
    y += 20;
    
    m_economyLabel = CreateWindowW(L"STATIC", L"100",
                                    WS_VISIBLE | SS_LEFT, panelX, y, 150, 20,
                                    m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_economyLabel, WM_SETFONT, (WPARAM)hSmallFont, 0);
    y += 20;
    
    m_economyBar = CreateWindowW(PROGRESS_CLASSW, nullptr,
                                  WS_VISIBLE | PBS_SMOOTH, panelX, y, 250, 18,
                                  m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_economyBar, PBM_SETRANGE, 0, MAKELPARAM(0, 1000));
    SendMessageW(m_economyBar, PBM_SETPOS, 100, 0);
    y += 30;

    // Technology
    HWND hTechTitle = CreateWindowW(L"STATIC", L"Технологии",
                                    WS_VISIBLE | SS_LEFT, panelX, y, 150, 20,
                                    m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hTechTitle, WM_SETFONT, (WPARAM)hFont, 0);
    y += 20;
    
    m_techLevelLabel = CreateWindowW(L"STATIC", L"Уровень: 0",
                                      WS_VISIBLE | SS_LEFT, panelX, y, 150, 20,
                                      m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_techLevelLabel, WM_SETFONT, (WPARAM)hSmallFont, 0);
    y += 20;
    
    m_techProgressBar = CreateWindowW(PROGRESS_CLASSW, nullptr,
                                       WS_VISIBLE | PBS_SMOOTH, panelX, y, 250, 18,
                                       m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_techProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    SendMessageW(m_techProgressBar, PBM_SETPOS, 0, 0);
    y += 40;

    // ===== RIGHT PANEL - Resources =====
    x = 320;
    y = 100;
    
    HWND hResTitle = CreateWindowW(L"STATIC", L"══════ РЕСУРСЫ ══════",
                                    WS_VISIBLE | SS_LEFT, x, y, 250, 25,
                                    m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hResTitle, WM_SETFONT, (WPARAM)hBoldFont, 0);
    y += 35;

    // Food
    HWND hFoodTitle = CreateWindowW(L"STATIC", L"  Еда",
                                     WS_VISIBLE | SS_LEFT, x, y, 150, 22,
                                     m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hFoodTitle, WM_SETFONT, (WPARAM)hFont, 0);
    m_foodLabel = CreateWindowW(L"STATIC", L"100",
                                 WS_VISIBLE | SS_RIGHT, x + 160, y, 100, 22,
                                 m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_foodLabel, WM_SETFONT, (WPARAM)hFont, 0);
    y += 30;

    // Money
    HWND hMoneyTitle = CreateWindowW(L"STATIC", L"  Деньги",
                                      WS_VISIBLE | SS_LEFT, x, y, 150, 22,
                                      m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hMoneyTitle, WM_SETFONT, (WPARAM)hFont, 0);
    m_moneyLabel = CreateWindowW(L"STATIC", L"500",
                                 WS_VISIBLE | SS_RIGHT, x + 160, y, 100, 22,
                                 m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_moneyLabel, WM_SETFONT, (WPARAM)hFont, 0);
    y += 30;

    // Energy
    HWND hEnergyTitle = CreateWindowW(L"STATIC", L"  Энергия",
                                      WS_VISIBLE | SS_LEFT, x, y, 150, 22,
                                      m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hEnergyTitle, WM_SETFONT, (WPARAM)hFont, 0);
    m_energyLabel = CreateWindowW(L"STATIC", L"50",
                                  WS_VISIBLE | SS_RIGHT, x + 160, y, 100, 22,
                                  m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_energyLabel, WM_SETFONT, (WPARAM)hFont, 0);
    y += 30;

    // Materials
    HWND hMatTitle = CreateWindowW(L"STATIC", L"  Материалы",
                                    WS_VISIBLE | SS_LEFT, x, y, 150, 22,
                                    m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hMatTitle, WM_SETFONT, (WPARAM)hFont, 0);
    m_materialsLabel = CreateWindowW(L"STATIC", L"100",
                                      WS_VISIBLE | SS_RIGHT, x + 160, y, 100, 22,
                                      m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_materialsLabel, WM_SETFONT, (WPARAM)hFont, 0);
    y += 50;

    // ===== Technology Panel =====
    HWND hTechPanelTitle = CreateWindowW(L"STATIC", L"══════ ТЕХНОЛОГИИ ══════",
                                          WS_VISIBLE | SS_LEFT, x, y, 250, 25,
                                          m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hTechPanelTitle, WM_SETFONT, (WPARAM)hBoldFont, 0);
    y += 35;

    m_techList = CreateWindowW(WC_LISTBOXW, nullptr,
                                WS_VISIBLE | WS_BORDER | LBS_NOTIFY | LBS_SORT,
                                x, y, 280, 200,
                                m_mainWindow, (HMENU)IDC_LIST_TECHS,
                                m_hInstance, nullptr);
    SendMessageW(m_techList, WM_SETFONT, (WPARAM)hSmallFont, 0);
    y += 220;

    // ===== Event Panel =====
    HWND hEventPanelTitle = CreateWindowW(L"STATIC", L"══════ СОБЫТИЯ ══════",
                                           WS_VISIBLE | SS_LEFT, x, y, 250, 25,
                                           m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hEventPanelTitle, WM_SETFONT, (WPARAM)hBoldFont, 0);
    y += 35;

    m_eventLabel = CreateWindowW(L"EDIT", L"",
                                  WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL,
                                  x, y, 280, 80,
                                  m_mainWindow, nullptr,
                                  m_hInstance, nullptr);
    SendMessageW(m_eventLabel, WM_SETFONT, (WPARAM)hSmallFont, 0);

    // ===== BOTTOM - Action Buttons =====
    y = 680;
    x = 20;
    
    m_nextTurnBtn = CreateWindowW(L"BUTTON", L"  Следующий ход  ",
                                   WS_VISIBLE | BS_PUSHBUTTON, x, y, 150, 45,
                                   m_mainWindow, (HMENU)IDC_NEXT_TURN,
                                   m_hInstance, nullptr);
    SendMessageW(m_nextTurnBtn, WM_SETFONT, (WPARAM)hBoldFont, 0);
    x += 160;

    m_investBtn = CreateWindowW(L"BUTTON", L"  Инвестировать  ",
                                 WS_VISIBLE | BS_PUSHBUTTON, x, y, 150, 45,
                                 m_mainWindow, (HMENU)IDC_INVEST,
                                 m_hInstance, nullptr);
    SendMessageW(m_investBtn, WM_SETFONT, (WPARAM)hBoldFont, 0);
    x += 160;

    m_researchBtn = CreateWindowW(L"BUTTON", L"  Исследовать  ",
                                   WS_VISIBLE | BS_PUSHBUTTON, x, y, 150, 45,
                                   m_mainWindow, (HMENU)IDC_RESEARCH,
                                   m_hInstance, nullptr);
    SendMessageW(m_researchBtn, WM_SETFONT, (WPARAM)hBoldFont, 0);
    x += 160;

    m_saveBtn = CreateWindowW(L"BUTTON", L"  Сохранить  ",
                               WS_VISIBLE | BS_PUSHBUTTON, x, y, 130, 45,
                               m_mainWindow, (HMENU)IDC_SAVE,
                               m_hInstance, nullptr);
    SendMessageW(m_saveBtn, WM_SETFONT, (WPARAM)hBoldFont, 0);
    x += 140;

    m_loadBtn = CreateWindowW(L"BUTTON", L"  Загрузить  ",
                               WS_VISIBLE | BS_PUSHBUTTON, x, y, 130, 45,
                               m_mainWindow, (HMENU)IDC_LOAD,
                               m_hInstance, nullptr);
    SendMessageW(m_loadBtn, WM_SETFONT, (WPARAM)hBoldFont, 0);
    x += 140;

    m_helpBtn = CreateWindowW(L"BUTTON", L"  Помощь  ",
                               WS_VISIBLE | BS_PUSHBUTTON, x, y, 120, 45,
                               m_mainWindow, (HMENU)IDC_GUI_HELP,
                               m_hInstance, nullptr);
    SendMessageW(m_helpBtn, WM_SETFONT, (WPARAM)hBoldFont, 0);
    x += 130;

    m_quitBtn = CreateWindowW(L"BUTTON", L"  Выход  ",
                              WS_VISIBLE | BS_PUSHBUTTON, x, y, 120, 45,
                              m_mainWindow, (HMENU)IDC_QUIT,
                              m_hInstance, nullptr);
    SendMessageW(m_quitBtn, WM_SETFONT, (WPARAM)hBoldFont, 0);
}

void Win32Gui::updateAllUI() {
    updateStats();
    updateResources();
    updateTechTree();
    updateEvents();
    updateEra();
}

void Win32Gui::updateStats() {
    if (!m_civ) return;

    SetWindowTextW(m_turnLabel, toWStr(u8"Ход: " + std::to_string(m_civ->getTurn())).c_str());
    
    int pop = m_civ->getPopulation();
    SetWindowTextW(m_populationLabel, toWStr(Utils::formatNumber(pop)).c_str());
    SendMessageW(m_populationBar, PBM_SETPOS, std::min(pop, 10000), 0);
    
    int happy = (int)m_civ->getHappiness();
    SetWindowTextW(m_happinessLabel, toWStr(std::to_string(happy) + "%").c_str());
    SendMessageW(m_happinessBar, PBM_SETPOS, happy, 0);
    
    int eco = (int)m_civ->getEcology();
    SetWindowTextW(m_ecologyLabel, toWStr(std::to_string(eco) + "%").c_str());
    SendMessageW(m_ecologyBar, PBM_SETPOS, eco, 0);
    
    int mil = (int)m_civ->getMilitary();
    SetWindowTextW(m_militaryLabel, toWStr(std::to_string(mil)).c_str());
    SendMessageW(m_militaryBar, PBM_SETPOS, std::min(mil, 100), 0);
    
    // Economy - use resources money as economy indicator
    double money = m_civ->getResources().getResource(ResourceType::Money);
    SetWindowTextW(m_economyLabel, toWStr(Utils::formatDouble(money, 0)).c_str());
    SendMessageW(m_economyBar, PBM_SETPOS, (int)std::min(money / 10.0, 500.0), 0);
    
    int tech = m_civ->getTech().getOverallTechLevel();
    SetWindowTextW(m_techLevelLabel, toWStr(u8"Уровень: " + std::to_string(tech) + "/100").c_str());
    SendMessageW(m_techProgressBar, PBM_SETPOS, tech, 0);
}

void Win32Gui::updateResources() {
    if (!m_civ) return;

    SetWindowTextW(m_foodLabel, toWStr(Utils::formatDouble(m_civ->getResources().getResource(ResourceType::Food), 0)).c_str());
    SetWindowTextW(m_moneyLabel, toWStr(Utils::formatDouble(m_civ->getResources().getResource(ResourceType::Money), 0)).c_str());
    SetWindowTextW(m_energyLabel, toWStr(Utils::formatDouble(m_civ->getResources().getResource(ResourceType::Energy), 0)).c_str());
    SetWindowTextW(m_materialsLabel, toWStr(Utils::formatDouble(m_civ->getResources().getResource(ResourceType::Materials), 0)).c_str());
}

void Win32Gui::updateTechTree() {
    if (!m_civ) {
        SendMessageW(m_techList, LB_RESETCONTENT, 0, 0);
        return;
    }

    SendMessageW(m_techList, LB_RESETCONTENT, 0, 0);

    auto researched = m_civ->getTech().getResearchedTechs();
    for (const auto* tech : researched) {
        std::wstring text = L"[x] " + toWStr(tech->name);
        SendMessageW(m_techList, LB_ADDSTRING, 0, (LPARAM)text.c_str());
    }

    auto available = m_civ->getTech().getAvailableTechs();
    for (const auto* tech : available) {
        std::wstringstream ss;
        ss << L"[ ] " << toWStr(tech->name) << L" (" << tech->cost << L")";
        SendMessageW(m_techList, LB_ADDSTRING, 0, (LPARAM)ss.str().c_str());
    }
}

void Win32Gui::updateEvents() {
    if (!m_civ) {
        SetWindowTextW(m_eventLabel, L"");
        return;
    }

    const auto& history = m_events->getEventHistory();
    if (history.empty()) {
        SetWindowTextW(m_eventLabel, L"Событий пока не было.");
        return;
    }

    std::wstringstream ss;
    size_t start = (history.size() > 5) ? history.size() - 5 : 0;
    for (size_t i = start; i < history.size(); ++i) {
        ss << L"[" << (i + 1) << L"] " << toWStr(history[i].name) << L"\r\n";
    }
    SetWindowTextW(m_eventLabel, ss.str().c_str());
}

void Win32Gui::updateEra() {
    if (!m_civ) return;
    SetWindowTextW(m_eraLabel, toWStr(u8"Эпоха: " + eraToString(m_civ->getCurrentEra())).c_str());
}

void Win32Gui::onNextTurn() {
    if (!m_civ) {
        // Start new game
        m_civ = std::make_unique<Civilization>(u8"Цивилизация");
        m_events = std::make_unique<EventSystem>();
        m_saveSystem = std::make_unique<SaveSystem>();
        m_events->init(m_difficulty);
    }

    GameEvent event = m_events->generateEvent(m_civ->getCurrentEra(), m_civ->getTurn());
    m_events->recordEvent(event);
    m_civ->applyEvent(event);
    m_civ->processTurn();

    std::wstringstream ss;
    ss << toWStr(event.name) << L"\r\n\r\n" << toWStr(event.description);
    SetWindowTextW(m_eventLabel, ss.str().c_str());

    updateAllUI();
}

void Win32Gui::onInvest() {
    if (!m_civ) {
        showMessage(L"Ошибка", L"Сначала начните новую игру!");
        return;
    }
    showMessage(L"Инвестиции", L"Выберите ветку технологий и введите сумму инвестиций.");
}

void Win32Gui::onResearch() {
    if (!m_civ) {
        showMessage(L"Ошибка", L"Сначала начните новую игру!");
        return;
    }
    showMessage(L"Исследование", L"Выберите технологию из списка для исследования.");
}

void Win32Gui::onSave() {
    if (!m_civ) {
        showMessage(L"Ошибка", L"Сначала начните новую игру!");
        return;
    }
    if (m_saveSystem->saveGame(*m_civ, *m_events, m_difficulty)) {
        showMessage(L"Сохранение", L"Игра успешно сохранена!");
    } else {
        showMessage(L"Ошибка", toWStr(m_saveSystem->getLastError()).c_str());
    }
}

void Win32Gui::onLoad() {
    if (!SaveSystem::saveExists()) {
        showMessage(L"Ошибка", L"Файл сохранения не найден!");
        return;
    }
    
    m_civ = std::make_unique<Civilization>();
    m_events = std::make_unique<EventSystem>();
    m_saveSystem = std::make_unique<SaveSystem>();
    
    if (m_saveSystem->loadGame(*m_civ, *m_events, m_difficulty)) {
        showMessage(L"Загрузка", L"Игра успешно загружена!");
        updateAllUI();
    } else {
        showMessage(L"Ошибка", toWStr(m_saveSystem->getLastError()).c_str());
    }
}

void Win32Gui::onHelp() {
    showMessage(L"Помощь",
        L"ЦЕЛЬ: Проведите цивилизацию от Каменного века до Космической эры!\n\n"
        L"УСЛОВИЯ ПОБЕДЫ:\n"
        L"- Освоение космоса (макс. уровень космических технологий)\n"
        L"- Экономическая стабильность (50 ходов подряд)\n"
        L"- Технологическое превосходство (уровень 100)\n\n"
        L"УСЛОВИЯ ПОРАЖЕНИЯ:\n"
        L"- Население достигло 0\n"
        L"- Экология ниже 5%\n"
        L"- Экономический коллапс\n\n"
        L"УПРАВЛЕНИЕ:\n"
        L"- Следующий ход - продвигает время\n"
        L"- Инвестиции - вкладывайте деньги в технологии\n"
        L"- Исследование - открывайте новые технологии"
    );
}

void Win32Gui::onQuit() {
    if (MessageBoxW(m_mainWindow, L"Вы уверены, что хотите выйти?", L"Выход",
                    MB_YESNO | MB_ICONQUESTION) == IDYES) {
        m_running = false;
        PostQuitMessage(0);
    }
}

void Win32Gui::onDifficultyChange() {
    int idx = SendMessageW(m_difficultyCombo, CB_GETCURSEL, 0, 0);
    m_difficulty = static_cast<Difficulty>(idx);
}

void Win32Gui::onBranchChange() {
    // Handle branch selection
}

void Win32Gui::onTechSelect() {
    // Handle tech selection from list
}

void Win32Gui::showMessage(const std::wstring& title, const std::wstring& text) {
    MessageBoxW(m_mainWindow, text.c_str(), title.c_str(), MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK Win32Gui::StaticWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Win32Gui* pThis = nullptr;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCTW* pCreate = (CREATESTRUCTW*)lParam;
        pThis = (Win32Gui*)pCreate->lpCreateParams;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
    } else {
        pThis = (Win32Gui*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    }

    if (pThis) {
        return pThis->WndProc(hwnd, uMsg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

LRESULT Win32Gui::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_NEXT_TURN: onNextTurn(); break;
                case IDC_INVEST: onInvest(); break;
                case IDC_RESEARCH: onResearch(); break;
                case IDC_SAVE: onSave(); break;
                case IDC_LOAD: onLoad(); break;
                case IDC_GUI_HELP: onHelp(); break;
                case IDC_QUIT: onQuit(); break;
                case IDC_COMBO_DIFFICULTY: onDifficultyChange(); break;
                case IDC_COMBO_BRANCH: onBranchChange(); break;
                case IDC_LIST_TECHS: if (HIWORD(wParam) == LBN_DBLCLK) onTechSelect(); break;
            }
            break;
        case WM_DESTROY:
            m_running = false;
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

} // namespace civ
