#include "ui/Win32Gui.h"
#include "core/Utils.h"
#include <Windows.h>
#include <commctrl.h>
#include <string>
#include <sstream>

#pragma comment(lib, "comctl32.lib")

namespace civ {

// Colors for UI elements
const COLORREF BG_COLOR = RGB(240, 242, 245);
const COLORREF PANEL_BG = RGB(255, 255, 255);
const COLORREF ACCENT_BLUE = RGB(52, 152, 219);
const COLORREF ACCENT_GREEN = RGB(46, 204, 113);
const COLORREF ACCENT_RED = RGB(231, 76, 60);
const COLORREF ACCENT_ORANGE = RGB(241, 196, 15);
const COLORREF ACCENT_PURPLE = RGB(155, 89, 182);

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
    icex.dwICC = ICC_PROGRESS_CLASS | ICC_LISTVIEW_CLASSES;
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
        0, CLASS_NAME, WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
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
    HFONT hTitleFont = CreateFontW(26, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                    DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                                    CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                    VARIABLE_PITCH, L"Segoe UI");
    HFONT hHeaderFont = CreateFontW(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                    DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                                    CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                    VARIABLE_PITCH, L"Segoe UI");
    HFONT hLabelFont = CreateFontW(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                                   CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                   VARIABLE_PITCH, L"Segoe UI");
    HFONT hValueFont = CreateFontW(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                    DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                                    CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                    VARIABLE_PITCH, L"Consolas");
    HFONT hBtnFont = CreateFontW(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                                   CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                   VARIABLE_PITCH, L"Segoe UI");

    int x = 20, y = 20;

    // ===== TITLE =====
    HWND hTitle = CreateWindowW(L"STATIC", L"🏛️ СИМУЛЯТОР ЦИВИЛИЗАЦИИ",
                                 WS_VISIBLE | SS_LEFT, x, y, 400, 40,
                                 m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hTitle, WM_SETFONT, (WPARAM)hTitleFont, 0);
    y += 50;

    // Era and Turn on the right
    m_eraLabel = CreateWindowW(L"STATIC", L"Каменный век",
                                WS_VISIBLE | SS_LEFT, 650, y, 400, 30,
                                m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_eraLabel, WM_SETFONT, (WPARAM)hHeaderFont, 0);
    y += 35;

    m_turnLabel = CreateWindowW(L"STATIC", L"Ход: 0",
                                WS_VISIBLE | SS_LEFT, 650, y + 5, 200, 22,
                                m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_turnLabel, WM_SETFONT, (WPARAM)hValueFont, 0);
    y += 40;

    // ===== TOP ROW: Stats Panels =====
    int panelY = y;
    int panelW = 250;
    int panelH = 180;
    int panelGap = 20;

    // ===== POPULATION PANEL =====
    HWND hPopBg = CreateWindowW(L"STATIC", nullptr,
                                 WS_VISIBLE | SS_LEFT | WS_BORDER,
                                 x, panelY, panelW, panelH,
                                 m_mainWindow, nullptr, m_hInstance, nullptr);
    
    HWND hPopTitle = CreateWindowW(L"STATIC", L"👥 НАСЕЛЕНИЕ",
                                    WS_VISIBLE | SS_LEFT, x + 10, panelY + 10, 230, 25,
                                    m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hPopTitle, WM_SETFONT, (WPARAM)hLabelFont, 0);

    m_populationLabel = CreateWindowW(L"STATIC", L"1 000",
                                       WS_VISIBLE | SS_LEFT | SS_CENTER, x + 10, panelY + 45, 230, 30,
                                       m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_populationLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    m_populationBar = CreateWindowW(PROGRESS_CLASSW, nullptr,
                                     WS_VISIBLE | PBS_SMOOTH, x + 15, panelY + 85, 220, 20,
                                     m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_populationBar, PBM_SETRANGE, 0, MAKELPARAM(0, 10000));
    SendMessageW(m_populationBar, PBM_SETPOS, 1000, 0);

    HWND hPopDesc = CreateWindowW(L"STATIC", L"Рост зависит от счастья",
                                    WS_VISIBLE | SS_LEFT | SS_CENTER, x + 10, panelY + 115, 230, 20,
                                    m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hPopDesc, WM_SETFONT, (WPARAM)hValueFont, 0);

    // ===== HAPPINESS PANEL =====
    x += panelW + panelGap;
    HWND hHappyBg = CreateWindowW(L"STATIC", nullptr,
                                   WS_VISIBLE | SS_LEFT | WS_BORDER,
                                   x, panelY, panelW, panelH,
                                   m_mainWindow, nullptr, m_hInstance, nullptr);

    HWND hHappyTitle = CreateWindowW(L"STATIC", L"😊 СЧАСТЬЕ",
                                      WS_VISIBLE | SS_LEFT, x + 10, panelY + 10, 230, 25,
                                      m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hHappyTitle, WM_SETFONT, (WPARAM)hLabelFont, 0);

    m_happinessLabel = CreateWindowW(L"STATIC", L"70%",
                                      WS_VISIBLE | SS_LEFT | SS_CENTER, x + 10, panelY + 45, 230, 30,
                                      m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_happinessLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    m_happinessBar = CreateWindowW(PROGRESS_CLASSW, nullptr,
                                    WS_VISIBLE | PBS_SMOOTH, x + 15, panelY + 85, 220, 20,
                                    m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_happinessBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    SendMessageW(m_happinessBar, PBM_SETPOS, 70, 0);

    HWND hHappyDesc = CreateWindowW(L"STATIC", L"Влияет на рост населения",
                                     WS_VISIBLE | SS_LEFT | SS_CENTER, x + 10, panelY + 115, 230, 20,
                                     m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hHappyDesc, WM_SETFONT, (WPARAM)hValueFont, 0);

    // ===== ECOLOGY PANEL =====
    x += panelW + panelGap;
    HWND hEcoBg = CreateWindowW(L"STATIC", nullptr,
                                 WS_VISIBLE | SS_LEFT | WS_BORDER,
                                 x, panelY, panelW, panelH,
                                 m_mainWindow, nullptr, m_hInstance, nullptr);

    HWND hEcoTitle = CreateWindowW(L"STATIC", L"🌿 ЭКОЛОГИЯ",
                                   WS_VISIBLE | SS_LEFT, x + 10, panelY + 10, 230, 25,
                                   m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hEcoTitle, WM_SETFONT, (WPARAM)hLabelFont, 0);

    m_ecologyLabel = CreateWindowW(L"STATIC", L"90%",
                                   WS_VISIBLE | SS_LEFT | SS_CENTER, x + 10, panelY + 45, 230, 30,
                                   m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_ecologyLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    m_ecologyBar = CreateWindowW(PROGRESS_CLASSW, nullptr,
                                  WS_VISIBLE | PBS_SMOOTH, x + 15, panelY + 85, 220, 20,
                                  m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_ecologyBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    SendMessageW(m_ecologyBar, PBM_SETPOS, 90, 0);

    HWND hEcoDesc = CreateWindowW(L"STATIC", L"Ниже 5% = поражение",
                                   WS_VISIBLE | SS_LEFT | SS_CENTER, x + 10, panelY + 115, 230, 20,
                                   m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hEcoDesc, WM_SETFONT, (WPARAM)hValueFont, 0);

    // ===== MILITARY PANEL =====
    x += panelW + panelGap;
    HWND hMilBg = CreateWindowW(L"STATIC", nullptr,
                                 WS_VISIBLE | SS_LEFT | WS_BORDER,
                                 x, panelY, panelW, panelH,
                                 m_mainWindow, nullptr, m_hInstance, nullptr);

    HWND hMilTitle = CreateWindowW(L"STATIC", L"⚔️ АРМИЯ",
                                    WS_VISIBLE | SS_LEFT, x + 10, panelY + 10, 230, 25,
                                    m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hMilTitle, WM_SETFONT, (WPARAM)hLabelFont, 0);

    m_militaryLabel = CreateWindowW(L"STATIC", L"10",
                                    WS_VISIBLE | SS_LEFT | SS_CENTER, x + 10, panelY + 45, 230, 30,
                                    m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_militaryLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    m_militaryBar = CreateWindowW(PROGRESS_CLASSW, nullptr,
                                   WS_VISIBLE | PBS_SMOOTH, x + 15, panelY + 85, 220, 20,
                                   m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_militaryBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    SendMessageW(m_militaryBar, PBM_SETPOS, 10, 0);

    HWND hMilDesc = CreateWindowW(L"STATIC", L"Защищает от войн",
                                   WS_VISIBLE | SS_LEFT | SS_CENTER, x + 10, panelY + 115, 230, 20,
                                   m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hMilDesc, WM_SETFONT, (WPARAM)hValueFont, 0);

    // ===== TECHNOLOGY PANEL =====
    x += panelW + panelGap;
    HWND hTechBg = CreateWindowW(L"STATIC", nullptr,
                                  WS_VISIBLE | SS_LEFT | WS_BORDER,
                                  x, panelY, panelW, panelH,
                                  m_mainWindow, nullptr, m_hInstance, nullptr);

    HWND hTechTitle = CreateWindowW(L"STATIC", L"🔬 ТЕХНОЛОГИИ",
                                     WS_VISIBLE | SS_LEFT, x + 10, panelY + 10, 230, 25,
                                     m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hTechTitle, WM_SETFONT, (WPARAM)hLabelFont, 0);

    m_techLabel = CreateWindowW(L"STATIC", L"Уровень: 0/100",
                                WS_VISIBLE | SS_LEFT | SS_CENTER, x + 10, panelY + 45, 230, 30,
                                m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_techLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    m_techBar = CreateWindowW(PROGRESS_CLASSW, nullptr,
                               WS_VISIBLE | PBS_SMOOTH, x + 15, panelY + 85, 220, 20,
                               m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_techBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    SendMessageW(m_techBar, PBM_SETPOS, 0, 0);

    HWND hTechDesc = CreateWindowW(L"STATIC", L"100 = победа!",
                                    WS_VISIBLE | SS_LEFT | SS_CENTER, x + 10, panelY + 115, 230, 20,
                                    m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hTechDesc, WM_SETFONT, (WPARAM)hValueFont, 0);

    // ===== MIDDLE ROW: Resources and Tech =====
    y = panelY + panelH + 25;
    x = 20;

    // ===== RESOURCES PANEL =====
    HWND hResBg = CreateWindowW(L"STATIC", nullptr,
                                 WS_VISIBLE | SS_LEFT | WS_BORDER,
                                 x, y, 350, 180,
                                 m_mainWindow, nullptr, m_hInstance, nullptr);

    HWND hResTitle = CreateWindowW(L"STATIC", L"💰 РЕСУРСЫ",
                                    WS_VISIBLE | SS_LEFT, x + 15, y + 12, 320, 28,
                                    m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hResTitle, WM_SETFONT, (WPARAM)hHeaderFont, 0);
    y += 45;

    // Food
    HWND hFoodIcon = CreateWindowW(L"STATIC", L"🍖",
                                    WS_VISIBLE | SS_LEFT, x + 20, y, 40, 30,
                                    m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hFoodIcon, WM_SETFONT, (WPARAM)hValueFont, 0);
    HWND hFoodTitle = CreateWindowW(L"STATIC", L"Еда",
                                     WS_VISIBLE | SS_LEFT, x + 60, y + 5, 80, 25,
                                     m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hFoodTitle, WM_SETFONT, (WPARAM)hLabelFont, 0);
    m_foodLabel = CreateWindowW(L"STATIC", L"100",
                                 WS_VISIBLE | SS_RIGHT, x + 150, y, 160, 25,
                                 m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_foodLabel, WM_SETFONT, (WPARAM)hValueFont, 0);
    y += 35;

    // Money
    HWND hMoneyIcon = CreateWindowW(L"STATIC", L"💵",
                                     WS_VISIBLE | SS_LEFT, x + 20, y, 40, 30,
                                     m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hMoneyIcon, WM_SETFONT, (WPARAM)hValueFont, 0);
    HWND hMoneyTitle = CreateWindowW(L"STATIC", L"Деньги",
                                     WS_VISIBLE | SS_LEFT, x + 60, y + 5, 80, 25,
                                     m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hMoneyTitle, WM_SETFONT, (WPARAM)hLabelFont, 0);
    m_moneyLabel = CreateWindowW(L"STATIC", L"500",
                                  WS_VISIBLE | SS_RIGHT, x + 150, y, 160, 25,
                                  m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_moneyLabel, WM_SETFONT, (WPARAM)hValueFont, 0);
    y += 35;

    // Energy
    HWND hEnergyIcon = CreateWindowW(L"STATIC", L"⚡",
                                      WS_VISIBLE | SS_LEFT, x + 20, y, 40, 30,
                                      m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hEnergyIcon, WM_SETFONT, (WPARAM)hValueFont, 0);
    HWND hEnergyTitle = CreateWindowW(L"STATIC", L"Энергия",
                                       WS_VISIBLE | SS_LEFT, x + 60, y + 5, 80, 25,
                                       m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hEnergyTitle, WM_SETFONT, (WPARAM)hLabelFont, 0);
    m_energyLabel = CreateWindowW(L"STATIC", L"50",
                                  WS_VISIBLE | SS_RIGHT, x + 150, y, 160, 25,
                                  m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_energyLabel, WM_SETFONT, (WPARAM)hValueFont, 0);
    y += 35;

    // Materials
    HWND hMatIcon = CreateWindowW(L"STATIC", L"🔩",
                                    WS_VISIBLE | SS_LEFT, x + 20, y, 40, 30,
                                    m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hMatIcon, WM_SETFONT, (WPARAM)hValueFont, 0);
    HWND hMatTitle = CreateWindowW(L"STATIC", L"Материалы",
                                    WS_VISIBLE | SS_LEFT, x + 60, y + 5, 80, 25,
                                    m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hMatTitle, WM_SETFONT, (WPARAM)hLabelFont, 0);
    m_materialsLabel = CreateWindowW(L"STATIC", L"100",
                                      WS_VISIBLE | SS_RIGHT, x + 150, y, 160, 25,
                                      m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_materialsLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    // ===== TECHNOLOGY LIST PANEL =====
    x = 390;
    y = panelY + panelH + 25;

    HWND hTechListBg = CreateWindowW(L"STATIC", nullptr,
                                      WS_VISIBLE | SS_LEFT | WS_BORDER,
                                      x, y, 320, 180,
                                      m_mainWindow, nullptr, m_hInstance, nullptr);

    HWND hTechListTitle = CreateWindowW(L"STATIC", L"🔧 ТЕХНОЛОГИИ",
                                         WS_VISIBLE | SS_LEFT, x + 15, y + 12, 290, 28,
                                         m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hTechListTitle, WM_SETFONT, (WPARAM)hHeaderFont, 0);

    m_techList = CreateWindowW(WC_LISTBOXW, nullptr,
                                WS_VISIBLE | WS_BORDER | LBS_NOTIFY | LBS_SORT | LBS_USETABSTOPS,
                                x + 15, y + 50, 290, 115,
                                m_mainWindow, (HMENU)IDC_LIST_TECHS,
                                m_hInstance, nullptr);
    SendMessageW(m_techList, WM_SETFONT, (WPARAM)hValueFont, 0);

    // ===== EVENT PANEL =====
    x = 730;
    y = panelY + panelH + 25;

    HWND hEventBg = CreateWindowW(L"STATIC", nullptr,
                                   WS_VISIBLE | SS_LEFT | WS_BORDER,
                                   x, y, 330, 180,
                                   m_mainWindow, nullptr, m_hInstance, nullptr);

    HWND hEventTitle = CreateWindowW(L"STATIC", L"📜 ПОСЛЕДНИЕ СОБЫТИЯ",
                                      WS_VISIBLE | SS_LEFT, x + 15, y + 12, 300, 28,
                                      m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hEventTitle, WM_SETFONT, (WPARAM)hHeaderFont, 0);

    m_eventLabel = CreateWindowW(L"EDIT", L"Нажмите 'Следующий ход' для начала игры!",
                                  WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL,
                                  x + 15, y + 50, 300, 115,
                                  m_mainWindow, nullptr,
                                  m_hInstance, nullptr);
    SendMessageW(m_eventLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    // ===== BOTTOM: Action Buttons =====
    y = 570;
    x = 20;

    m_nextTurnBtn = CreateWindowW(L"BUTTON", L"🎲 СЛЕДУЮЩИЙ ХОД",
                                   WS_VISIBLE | BS_PUSHBUTTON, x, y, 200, 50,
                                   m_mainWindow, (HMENU)IDC_BTN_NEXT,
                                   m_hInstance, nullptr);
    SendMessageW(m_nextTurnBtn, WM_SETFONT, (WPARAM)hBtnFont, 0);
    SendMessageW(m_nextTurnBtn, BCM_SETSHIELD, 0, TRUE);
    x += 215;

    m_investBtn = CreateWindowW(L"BUTTON", L"💰 ИНВЕСТИРОВАТЬ",
                                 WS_VISIBLE | BS_PUSHBUTTON, x, y, 180, 50,
                                 m_mainWindow, (HMENU)IDC_BTN_INVEST,
                                 m_hInstance, nullptr);
    SendMessageW(m_investBtn, WM_SETFONT, (WPARAM)hBtnFont, 0);
    x += 195;

    m_researchBtn = CreateWindowW(L"BUTTON", L"🔬 ИССЛЕДОВАТЬ",
                                   WS_VISIBLE | BS_PUSHBUTTON, x, y, 180, 50,
                                   m_mainWindow, (HMENU)IDC_BTN_RESEARCH,
                                   m_hInstance, nullptr);
    SendMessageW(m_researchBtn, WM_SETFONT, (WPARAM)hBtnFont, 0);
    x += 195;

    m_saveBtn = CreateWindowW(L"BUTTON", L"💾 СОХРАНИТЬ",
                               WS_VISIBLE | BS_PUSHBUTTON, x, y, 150, 50,
                               m_mainWindow, (HMENU)IDC_BTN_SAVE,
                               m_hInstance, nullptr);
    SendMessageW(m_saveBtn, WM_SETFONT, (WPARAM)hBtnFont, 0);
    x += 165;

    m_loadBtn = CreateWindowW(L"BUTTON", L"📂 ЗАГРУЗИТЬ",
                               WS_VISIBLE | BS_PUSHBUTTON, x, y, 150, 50,
                               m_mainWindow, (HMENU)IDC_BTN_LOAD,
                               m_hInstance, nullptr);
    SendMessageW(m_loadBtn, WM_SETFONT, (WPARAM)hBtnFont, 0);
    x += 165;

    m_helpBtn = CreateWindowW(L"BUTTON", L"❓ ПОМОЩЬ",
                               WS_VISIBLE | BS_PUSHBUTTON, x, y, 130, 50,
                               m_mainWindow, (HMENU)IDC_BTN_HELP,
                               m_hInstance, nullptr);
    SendMessageW(m_helpBtn, WM_SETFONT, (WPARAM)hBtnFont, 0);
    x += 145;

    m_quitBtn = CreateWindowW(L"BUTTON", L"🚪 ВЫХОД",
                              WS_VISIBLE | BS_PUSHBUTTON, x, y, 120, 50,
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
    
    int tech = m_civ->getTech().getOverallTechLevel();
    SetWindowTextW(m_techLabel, toWStr(u8"Уровень: " + std::to_string(tech) + "/100").c_str());
    SendMessageW(m_techBar, PBM_SETPOS, tech, 0);
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
        ss << L"[ ] " << toWStr(tech->name) << L" (" << tech->cost << L")";
        SendMessageW(m_techList, LB_ADDSTRING, 0, (LPARAM)ss.str().c_str());
    }
}

void Win32Gui::updateEvents() {
    if (!m_civ) {
        return;
    }

    const auto& history = m_events->getEventHistory();
    if (history.empty()) {
        return;
    }

    std::wstringstream ss;
    size_t start = (history.size() > 5) ? history.size() - 5 : 0;
    for (size_t i = start; i < history.size(); ++i) {
        ss << L"▸ " << toWStr(history[i].name) << L"\r\n";
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

    std::wstringstream ss;
    ss << L"━━━━━━━━━━━━━━━━━━━━━━━━\r\n";
    ss << L"📌 " << toWStr(event.name) << L"\r\n\r\n";
    ss << toWStr(event.description) << L"\r\n";
    ss << L"━━━━━━━━━━━━━━━━━━━━━━━━";
    SetWindowTextW(m_eventLabel, ss.str().c_str());

    updateAllUI();
}

void Win32Gui::onInvest() {
    if (!m_civ) {
        MessageBoxW(m_mainWindow, L"Сначала начните новую игру!", L"Ошибка", MB_OK | MB_ICONWARNING);
        return;
    }
    MessageBoxW(m_mainWindow, 
        L"Выберите ветку технологий из меню исследования\nи вложите деньги в её развитие.",
        L"Инвестирование", MB_OK | MB_ICONINFORMATION);
}

void Win32Gui::onResearch() {
    if (!m_civ) {
        MessageBoxW(m_mainWindow, L"Сначала начните новую игру!", L"Ошибка", MB_OK | MB_ICONWARNING);
        return;
    }
    MessageBoxW(m_mainWindow, 
        L"Дважды кликните по технологии в списке,\nчтобы исследовать её (требуются деньги).",
        L"Исследование технологий", MB_OK | MB_ICONINFORMATION);
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
        L"🏛️ СИМУЛЯТОР ЦИВИЛИЗАЦИИ\n\n"
        L"━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n"
        L"🎯 ЦЕЛЬ:\n"
        L"Доведите цивилизацию от Каменного века до Космической эры!\n\n"
        L"✅ УСЛОВИЯ ПОБЕДЫ:\n"
        L"• Освоение космоса (макс. уровень космических технологий)\n"
        L"• Экономическая стабильность (50 ходов подряд)\n"
        L"• Технологическое превосходство (уровень 100)\n\n"
        L"❌ УСЛОВИЯ ПОРАЖЕНИЯ:\n"
        L"• Население достигло 0\n"
        L"• Экология ниже 5%\n"
        L"• Экономический коллапс\n\n"
        L"💡 СОВЕТЫ:\n"
        L"• Балансируйте инвестиции между ветками\n"
        L"• Наука улучшает экологию\n"
        L"• Медицина увеличивает рост населения\n"
        L"• Поддерживайте высокое счастье",
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
    // Future: Implement tech research from list
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
