#define NOMINMAX

#include "ui/Win32Gui.h"
#include "core/Utils.h"
#include <Windows.h>
#include <commctrl.h>
#include <string>
#include <sstream>
#include <algorithm>

#pragma comment(lib, "comctl32.lib")

namespace civ {

// Colors for UI elements
const COLORREF BG_COLOR = RGB(240, 242, 245);
const COLORREF PANEL_BG = RGB(255, 255, 255);
const COLORREF TEXT_COLOR = RGB(30, 30, 30);

Win32Gui::Win32Gui()
    : m_hInstance(nullptr)
    , m_mainWindow(nullptr)
    , m_running(false)
    , m_difficulty(Difficulty::Normal)
    , m_bgBrush(nullptr)
    , m_panelBrush(nullptr)
{
}

Win32Gui::~Win32Gui() {
    if (m_bgBrush) DeleteObject(m_bgBrush);
    if (m_panelBrush) DeleteObject(m_panelBrush);
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
    icex.dwICC = ICC_PROGRESS_CLASS | ICC_LISTVIEW_CLASSES | ICC_UPDOWN_CLASS;
    InitCommonControlsEx(&icex);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = StaticWndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_HAND);
    wc.hbrBackground = CreateSolidBrush(BG_COLOR);
    wc.lpszClassName = CLASS_NAME;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

    if (!RegisterClassExW(&wc)) {
        return 1;
    }

    m_mainWindow = CreateWindowExW(
        WS_EX_COMPOSITED, CLASS_NAME, WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr, nullptr, hInstance, this
    );

    if (!m_mainWindow) {
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
    // Create brushes
    m_bgBrush = CreateSolidBrush(BG_COLOR);
    m_panelBrush = CreateSolidBrush(PANEL_BG);

    // Fonts
    HFONT hTitleFont = CreateFontW(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                    DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                                    CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                    VARIABLE_PITCH, L"Segoe UI");
    HFONT hBtnFont = CreateFontW(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                                   CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                   VARIABLE_PITCH, L"Segoe UI");
    HFONT hLabelFont = CreateFontW(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                                   CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                   VARIABLE_PITCH, L"Segoe UI");
    HFONT hValueFont = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                    DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                                    CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                    VARIABLE_PITCH, L"Consolas");

    int childStyle = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS;
    int btnStyle = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | BS_PUSHBUTTON;

    int x = 20, y = 20;

    // Title
    HWND hTitle = CreateWindowW(L"STATIC", L"СИМУЛЯТОР ЦИВИЛИЗАЦИИ",
                                 childStyle | SS_LEFT, x, y, 300, 35,
                                 m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hTitle, WM_SETFONT, (WPARAM)hTitleFont, 0);

    // Era and Turn
    y = 65;
    m_eraLabel = CreateWindowW(L"STATIC", L"Каменный век",
                                childStyle | SS_LEFT, x, y, 200, 25,
                                m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_eraLabel, WM_SETFONT, (WPARAM)hLabelFont, 0);

    m_turnLabel = CreateWindowW(L"STATIC", L"Ход: 0",
                                childStyle | SS_LEFT, x + 220, y, 100, 25,
                                m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_turnLabel, WM_SETFONT, (WPARAM)hLabelFont, 0);

    // Stats in one row - compact
    y = 105;
    HWND hPopTitle = CreateWindowW(L"STATIC", L"Население:",
                  childStyle | SS_LEFT, x, y, 80, 20,
                  m_mainWindow, nullptr, m_hInstance, nullptr);
    
    m_populationLabel = CreateWindowW(L"STATIC", L"1000",
                                       childStyle | SS_LEFT, x + 90, y, 60, 20,
                                       m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_populationLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    HWND hHappyTitle = CreateWindowW(L"STATIC", L"Счастье:",
                  childStyle | SS_LEFT, x + 170, y, 70, 20,
                  m_mainWindow, nullptr, m_hInstance, nullptr);
    m_happinessLabel = CreateWindowW(L"STATIC", L"70%",
                                     childStyle | SS_LEFT, x + 250, y, 50, 20,
                                     m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_happinessLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    HWND hEcoTitle = CreateWindowW(L"STATIC", L"Экология:",
                  childStyle | SS_LEFT, x + 320, y, 70, 20,
                  m_mainWindow, nullptr, m_hInstance, nullptr);
    m_ecologyLabel = CreateWindowW(L"STATIC", L"90%",
                                    childStyle | SS_LEFT, x + 400, y, 50, 20,
                                    m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_ecologyLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    HWND hMilTitle = CreateWindowW(L"STATIC", L"Армия:",
                  childStyle | SS_LEFT, x + 470, y, 50, 20,
                  m_mainWindow, nullptr, m_hInstance, nullptr);
    m_militaryLabel = CreateWindowW(L"STATIC", L"10",
                                    childStyle | SS_LEFT, x + 530, y, 40, 20,
                                    m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_militaryLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    HWND hTechTitle = CreateWindowW(L"STATIC", L"Технологии:",
                  childStyle | SS_LEFT, x + 590, y, 90, 20,
                  m_mainWindow, nullptr, m_hInstance, nullptr);
    m_techLabel = CreateWindowW(L"STATIC", L"0/100",
                                childStyle | SS_LEFT, x + 690, y, 60, 20,
                                m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_techLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    // Resources row
    y = 140;
    HWND hFoodTitle = CreateWindowW(L"STATIC", L"Еда:",
                  childStyle | SS_LEFT, x, y, 40, 20,
                  m_mainWindow, nullptr, m_hInstance, nullptr);
    m_foodLabel = CreateWindowW(L"STATIC", L"100",
                                 childStyle | SS_LEFT, x + 50, y, 60, 20,
                                 m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_foodLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    HWND hMoneyTitle = CreateWindowW(L"STATIC", L"Деньги:",
                  childStyle | SS_LEFT, x + 130, y, 60, 20,
                  m_mainWindow, nullptr, m_hInstance, nullptr);
    m_moneyLabel = CreateWindowW(L"STATIC", L"500",
                                 childStyle | SS_LEFT, x + 200, y, 60, 20,
                                 m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_moneyLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    HWND hEnergyTitle = CreateWindowW(L"STATIC", L"Энергия:",
                  childStyle | SS_LEFT, x + 280, y, 60, 20,
                  m_mainWindow, nullptr, m_hInstance, nullptr);
    m_energyLabel = CreateWindowW(L"STATIC", L"50",
                                   childStyle | SS_LEFT, x + 350, y, 60, 20,
                                   m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_energyLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    HWND hMatTitle = CreateWindowW(L"STATIC", L"Материалы:",
                  childStyle | SS_LEFT, x + 430, y, 80, 20,
                  m_mainWindow, nullptr, m_hInstance, nullptr);
    m_materialsLabel = CreateWindowW(L"STATIC", L"100",
                                       childStyle | SS_LEFT, x + 520, y, 60, 20,
                                       m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_materialsLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    // Technologies List
    y = 180;
    HWND hTechListTitle = CreateWindowW(L"STATIC", L"Доступные технологии:",
                  childStyle | SS_LEFT, x, y - 25, 280, 20,
                  m_mainWindow, nullptr, m_hInstance, nullptr);
    
    m_techList = CreateWindowW(WC_LISTBOXW, nullptr,
                                childStyle | WS_BORDER | LBS_NOTIFY | LBS_SORT,
                                x, y, 350, 150,
                                m_mainWindow, (HMENU)IDC_LIST_TECHS,
                                m_hInstance, nullptr);
    SendMessageW(m_techList, WM_SETFONT, (WPARAM)hValueFont, 0);

    // Events panel (right side)
    x = 400;
    HWND hEventTitle = CreateWindowW(L"STATIC", L"События:",
                  childStyle | SS_LEFT, x, y - 25, 180, 20,
                  m_mainWindow, nullptr, m_hInstance, nullptr);

    m_eventLabel = CreateWindowW(L"EDIT", L"Нажмите 'Следующий ход' для начала игры!",
                                  childStyle | ES_MULTILINE | ES_READONLY | WS_VSCROLL,
                                  x, y, 350, 150,
                                  m_mainWindow, nullptr,
                                  m_hInstance, nullptr);
    SendMessageW(m_eventLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    // Action Buttons row
    y = 350;
    x = 20;

    m_nextTurnBtn = CreateWindowW(L"BUTTON", L"Следующий ход",
                                   btnStyle, x, y, 150, 45,
                                   m_mainWindow, (HMENU)IDC_BTN_NEXT,
                                   m_hInstance, nullptr);
    SendMessageW(m_nextTurnBtn, WM_SETFONT, (WPARAM)hBtnFont, 0);
    x += 165;

    m_investBtn = CreateWindowW(L"BUTTON", L"Инвестировать",
                                 btnStyle, x, y, 140, 45,
                                 m_mainWindow, (HMENU)IDC_BTN_INVEST,
                                 m_hInstance, nullptr);
    SendMessageW(m_investBtn, WM_SETFONT, (WPARAM)hBtnFont, 0);
    x += 155;

    m_researchBtn = CreateWindowW(L"BUTTON", L"Исследовать",
                                   btnStyle, x, y, 140, 45,
                                   m_mainWindow, (HMENU)IDC_BTN_RESEARCH,
                                   m_hInstance, nullptr);
    SendMessageW(m_researchBtn, WM_SETFONT, (WPARAM)hBtnFont, 0);
    x += 155;

    m_saveBtn = CreateWindowW(L"BUTTON", L"Сохранить",
                               btnStyle, x, y, 120, 45,
                               m_mainWindow, (HMENU)IDC_BTN_SAVE,
                               m_hInstance, nullptr);
    SendMessageW(m_saveBtn, WM_SETFONT, (WPARAM)hBtnFont, 0);
    x += 135;

    m_loadBtn = CreateWindowW(L"BUTTON", L"Загрузить",
                               btnStyle, x, y, 120, 45,
                               m_mainWindow, (HMENU)IDC_BTN_LOAD,
                               m_hInstance, nullptr);
    SendMessageW(m_loadBtn, WM_SETFONT, (WPARAM)hBtnFont, 0);
    x += 135;

    m_helpBtn = CreateWindowW(L"BUTTON", L"Помощь",
                               btnStyle, x, y, 100, 45,
                               m_mainWindow, (HMENU)IDC_BTN_HELP,
                               m_hInstance, nullptr);
    SendMessageW(m_helpBtn, WM_SETFONT, (WPARAM)hBtnFont, 0);
    x += 115;

    m_quitBtn = CreateWindowW(L"BUTTON", L"Выход",
                              btnStyle, x, y, 100, 45,
                              m_mainWindow, (HMENU)IDC_BTN_QUIT,
                              m_hInstance, nullptr);
    SendMessageW(m_quitBtn, WM_SETFONT, (WPARAM)hBtnFont, 0);
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
    
    int happy = (int)m_civ->getHappiness();
    SetWindowTextW(m_happinessLabel, toWStr(std::to_string(happy) + "%").c_str());
    
    int eco = (int)m_civ->getEcology();
    SetWindowTextW(m_ecologyLabel, toWStr(std::to_string(eco) + "%").c_str());
    
    int mil = (int)m_civ->getMilitary();
    SetWindowTextW(m_militaryLabel, toWStr(std::to_string(mil)).c_str());
    
    int tech = m_civ->getTech().getOverallTechLevel();
    SetWindowTextW(m_techLabel, toWStr(std::to_string(tech) + "/100").c_str());
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
        std::wstring text = L"[✓] " + toWStr(tech->name);
        SendMessageW(m_techList, LB_ADDSTRING, 0, (LPARAM)text.c_str());
    }

    auto available = m_civ->getTech().getAvailableTechs();
    for (const auto* tech : available) {
        std::wstringstream ss;
        ss << L"[ ] " << toWStr(tech->name) << L" ($" << tech->cost << L")";
        SendMessageW(m_techList, LB_ADDSTRING, 0, (LPARAM)ss.str().c_str());
    }
}

void Win32Gui::updateEvents() {
    if (!m_civ) {
        return;
    }

    const auto& history = m_events->getEventHistory();
    if (history.empty()) {
        SetWindowTextW(m_eventLabel, L"Нажмите 'Следующий ход' для начала игры!");
        return;
    }

    std::wstringstream ss;
    size_t start = (history.size() > 5) ? history.size() - 5 : 0;
    for (size_t i = start; i < history.size(); ++i) {
        ss << L"- " << toWStr(history[i].name) << L"\r\n";
    }
    SetWindowTextW(m_eventLabel, ss.str().c_str());
}

void Win32Gui::updateEra() {
    if (!m_civ) return;
    SetWindowTextW(m_eraLabel, toWStr(eraToString(m_civ->getCurrentEra())).c_str());
}

void Win32Gui::onNextTurn() {
    if (!m_civ) {
        m_civ = std::make_unique<Civilization>(u8"Цивилизация");
        m_events = std::make_unique<EventSystem>();
        m_saveSystem = std::make_unique<SaveSystem>();
        m_events->init(m_difficulty);
        updateEra();
        updateResources();
    }

    GameEvent event = m_events->generateEvent(m_civ->getCurrentEra(), m_civ->getTurn());
    m_events->recordEvent(event);
    m_civ->applyEvent(event);
    m_civ->processTurn();

    // Update events display with full info
    std::wstringstream ss;
    ss << L"=== СОБЫТИЕ ===\r\n";
    ss << toWStr(event.name) << L"\r\n\r\n";
    ss << toWStr(event.description) << L"\r\n";
    ss << L"\r\n=== ИСТОРИЯ ===\r\n";
    
    const auto& history = m_events->getEventHistory();
    size_t start = (history.size() > 5) ? history.size() - 5 : 0;
    for (size_t i = start; i < history.size(); ++i) {
        ss << L"- " << toWStr(history[i].name) << L"\r\n";
    }
    SetWindowTextW(m_eventLabel, ss.str().c_str());

    updateAllUI();
}

void Win32Gui::onInvest() {
    if (!m_civ) {
        MessageBoxW(m_mainWindow, L"Сначала начните новую игру!", L"Ошибка", MB_OK | MB_ICONWARNING);
        return;
    }
    
    int sel = (int)SendMessageW(m_techList, LB_GETCURSEL, 0, 0);
    if (sel == LB_ERR) {
        MessageBoxW(m_mainWindow, L"Выберите технологию из списка!", L"Ошибка", MB_OK | MB_ICONWARNING);
        return;
    }
    
    auto available = m_civ->getTech().getAvailableTechs();
    if (sel < 0 || sel >= (int)available.size()) {
        MessageBoxW(m_mainWindow, L"Нельзя инвестировать в исследованные технологии!", L"Ошибка", MB_OK | MB_ICONWARNING);
        return;
    }
    
    const auto* tech = available[sel];
    double money = m_civ->getResources().getResource(ResourceType::Money);
    
    if (money < tech->cost) {
        MessageBoxW(m_mainWindow, L"Недостаточно денег!", L"Ошибка", MB_OK | MB_ICONWARNING);
        return;
    }
    
    m_civ->getResources().addResource(ResourceType::Money, -tech->cost);
    m_civ->getTech().investInBranch(tech->branch, tech->cost * 0.5);
    
    updateResources();
    
    std::wstring msg = L"Инвестировано $" + std::to_wstring((int)(tech->cost * 0.5)) + L" в ветку " + toWStr(techBranchToString(tech->branch));
    MessageBoxW(m_mainWindow, msg.c_str(), L"Инвестирование", MB_OK | MB_ICONINFORMATION);
}

void Win32Gui::onResearch() {
    if (!m_civ) {
        MessageBoxW(m_mainWindow, L"Сначала начните новую игру!", L"Ошибка", MB_OK | MB_ICONWARNING);
        return;
    }
    
    int sel = (int)SendMessageW(m_techList, LB_GETCURSEL, 0, 0);
    if (sel == LB_ERR) {
        MessageBoxW(m_mainWindow, L"Выберите технологию из списка!", L"Ошибка", MB_OK | MB_ICONWARNING);
        return;
    }
    
    auto available = m_civ->getTech().getAvailableTechs();
    if (sel < 0 || sel >= (int)available.size()) {
        MessageBoxW(m_mainWindow, L"Эта технология уже исследована!", L"Ошибка", MB_OK | MB_ICONWARNING);
        return;
    }
    
    const auto* tech = available[sel];
    double money = m_civ->getResources().getResource(ResourceType::Money);
    
    if (money < tech->cost) {
        MessageBoxW(m_mainWindow, L"Недостаточно денег для исследования!", L"Ошибка", MB_OK | MB_ICONWARNING);
        return;
    }
    
    m_civ->getResources().addResource(ResourceType::Money, -tech->cost);
    m_civ->getTech().researchTech(tech->name);
    
    updateTechTree();
    updateResources();
    
    if (m_civ->getTech().getOverallTechLevel() >= 100) {
        MessageBoxW(m_mainWindow, L"ПОЗДРАВЛЯЕМ! Вы достигли макс. уровня технологий!", 
                    L"ПОБЕДА", MB_OK | MB_ICONINFORMATION);
    }
}

void Win32Gui::onSave() {
    if (!m_civ) {
        MessageBoxW(m_mainWindow, L"Сначала начните новую игру!", L"Ошибка", MB_OK | MB_ICONWARNING);
        return;
    }
    if (m_saveSystem->saveGame(*m_civ, *m_events, m_difficulty)) {
        MessageBoxW(m_mainWindow, L"Игра успешно сохранена!", L"Сохранение", MB_OK | MB_ICONINFORMATION);
    }
}

void Win32Gui::onLoad() {
    if (!SaveSystem::saveExists()) {
        MessageBoxW(m_mainWindow, L"Файл сохранения не найден!", L"Ошибка", MB_OK | MB_ICONWARNING);
        return;
    }
    
    m_civ = std::make_unique<Civilization>();
    m_events = std::make_unique<EventSystem>();
    m_saveSystem = std::make_unique<SaveSystem>();
    
    if (m_saveSystem->loadGame(*m_civ, *m_events, m_difficulty)) {
        MessageBoxW(m_mainWindow, L"Игра успешно загружена!", L"Загрузка", MB_OK | MB_ICONINFORMATION);
        updateAllUI();
    }
}

void Win32Gui::onHelpBtn() {
    MessageBoxW(m_mainWindow, 
        L"СИМУЛЯТОР ЦИВИЛИЗАЦИИ\n\n"
        L"ЦЕЛЬ: Доведите цивилизацию до уровня технологий 100!\n\n"
        L"УПРАВЛЕНИЕ:\n"
        L"- Следующий ход - продвинуть игру на 1 ход\n"
        L"- Исследовать - исследовать выбранную технологию\n"
        L"- Инвестировать - ускорить исследование ветки\n\n"
        L"СОВЕТЫ:\n"
        L"- Балансируйте ресурсы между едой и деньгами\n"
        L"- Наука улучшает экологию\n"
        L"- Поддерживайте высокое счастье населения",
        L"ПОМОЩЬ", MB_OK | MB_ICONQUESTION);
}

void Win32Gui::onQuit() {
    if (MessageBoxW(m_mainWindow, L"Вы уверены, что хотите выйти?", L"Выход",
                    MB_YESNO | MB_ICONQUESTION) == IDYES) {
        m_running = false;
        PostQuitMessage(0);
    }
}

void Win32Gui::onTechSelect() {
    onResearch();
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
                case IDC_BTN_NEXT: onNextTurn(); break;
                case IDC_BTN_INVEST: onInvest(); break;
                case IDC_BTN_RESEARCH: onResearch(); break;
                case IDC_BTN_SAVE: onSave(); break;
                case IDC_BTN_LOAD: onLoad(); break;
                case IDC_BTN_HELP: onHelpBtn(); break;
                case IDC_BTN_QUIT: onQuit(); break;
                case IDC_LIST_TECHS: 
                    if (HIWORD(wParam) == LBN_DBLCLK) onTechSelect(); 
                    break;
            }
            break;
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, TEXT_COLOR);
            SetBkColor(hdc, PANEL_BG);
            return (LRESULT)m_panelBrush;
        }
        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, m_bgBrush);
            return 1;
        }
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
