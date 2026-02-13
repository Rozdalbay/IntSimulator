#pragma once

#include <Windows.h>
#include "game/Civilization.h"
#include "game/EventSystem.h"
#include "game/SaveSystem.h"
#include <string>
#include <memory>

namespace civ {

constexpr LPCWSTR CLASS_NAME = L"IntSimulatorGUI";
constexpr LPCWSTR WINDOW_TITLE = L"Симулятор Эволюции Цивилизации";

constexpr int WINDOW_WIDTH = 1100;
constexpr int WINDOW_HEIGHT = 700;

// Control IDs (avoiding Windows IDC_* macros)
enum {
    IDC_BTN_NEXT = 1001,
    IDC_BTN_INVEST,
    IDC_BTN_RESEARCH,
    IDC_BTN_SAVE,
    IDC_BTN_LOAD,
    IDC_BTN_HELP,
    IDC_BTN_QUIT,
    IDC_LIST_TECHS,
};

class Win32Gui {
public:
    Win32Gui();
    ~Win32Gui();
    
    int run(HINSTANCE hInstance, int nCmdShow);

private:
    void createControls();
    void updateAllUI();
    void updateStats();
    void updateResources();
    void updateTechTree();
    void updateEvents();
    void updateEra();
    
    void onNextTurn();
    void onInvest();
    void onResearch();
    void onSave();
    void onLoad();
    void onHelpBtn();
    void onQuit();
    void onTechSelect();
    
    static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    std::wstring toWStr(const std::string& str);
    
    HINSTANCE m_hInstance;
    HWND m_mainWindow;
    bool m_running;
    
    // Brushes for background colors
    HBRUSH m_bgBrush = nullptr;
    HBRUSH m_panelBrush = nullptr;
    
    // Game state
    std::unique_ptr<Civilization> m_civ;
    std::unique_ptr<EventSystem> m_events;
    std::unique_ptr<SaveSystem> m_saveSystem;
    Difficulty m_difficulty;
    
    // Controls
    HWND m_eraLabel;
    HWND m_turnLabel;
    HWND m_populationLabel;
    HWND m_happinessLabel;
    HWND m_ecologyLabel;
    HWND m_militaryLabel;
    HWND m_techLabel;
    HWND m_populationBar;
    HWND m_happinessBar;
    HWND m_ecologyBar;
    HWND m_militaryBar;
    HWND m_techBar;
    
    HWND m_foodLabel;
    HWND m_moneyLabel;
    HWND m_energyLabel;
    HWND m_materialsLabel;
    
    HWND m_nextTurnBtn;
    HWND m_investBtn;
    HWND m_researchBtn;
    HWND m_saveBtn;
    HWND m_loadBtn;
    HWND m_helpBtn;
    HWND m_quitBtn;
    
    HWND m_techList;
    HWND m_eventLabel;
};

} // namespace civ
