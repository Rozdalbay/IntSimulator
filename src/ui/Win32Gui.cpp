#define NOMINMAX

#include "ui/Win32Gui.h"
#include "core/Utils.h"
#include "core/Types.h"
#include <Windows.h>
#include <commctrl.h>
#include <string>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <ctime>

#pragma comment(lib, "comctl32.lib")

namespace civ {

// Colors for UI elements
const COLORREF C7_BG = RGB(28, 32, 42);           // Main background
const COLORREF C7_PANEL = RGB(40, 45, 58);        // Panel background
const COLORREF C7_TEXT = RGB(220, 225, 230);      // Main text
const COLORREF C7_TEXT_DIM = RGB(160, 165, 170);  // Dimmed text for descriptions
const COLORREF C7_GOLD = RGB(200, 155, 60);       // Titles, highlights
const COLORREF C7_SELECTION = RGB(60, 68, 85);    // Listbox selection

// --- Building Definitions ---
struct BuildingDef {
    std::wstring name;
    double costMoney;
    double costMaterials;
    int popCapAdd;
    double foodAdd;
    double moneyAdd;
    double materialsAdd;
    double energyAdd;
    double militaryAdd;
    Era minEra;
    COLORREF color;
};

static const std::vector<BuildingDef> s_buildingDefs = {
    { L"Дом",       0,   50,  50, 0,   0,   0,   0,   0, Era::StoneAge,   RGB(200, 150, 100) },
    { L"Ферма",     20,  20,  0,  15,  0,   0,   0,   0, Era::StoneAge,   RGB(50, 200, 50) },
    { L"Рынок",     50,  50,  0,  0,   10,  0,   0,   0, Era::BronzeAge,  RGB(200, 200, 50) },
    { L"Шахта",     100, 0,   0,  0,   0,   10,  0,   0, Era::BronzeAge,  RGB(100, 100, 100) },
    { L"Казарма",   150, 100, 0,  0,   0,   0,   0,   5, Era::IronAge,    RGB(200, 50, 50) },
    { L"Завод",     300, 200, 0,  0,   20,  20, -10,  0, Era::Industrial, RGB(120, 120, 130) },
    { L"Эл.станция",400, 300, 0,  0,   0,   0,   30,  0, Era::Industrial, RGB(100, 200, 255) }
};

// --- City Map Visualizer ---
struct CityEntity {
    float x, y;
    float dx, dy;
    int type; // 0=Settler, 1=Building, 2=TownHall
    int variant;
    int buildingDefIdx; // Index in s_buildingDefs if type==1
    COLORREF color;
};

static std::vector<CityEntity> s_cityEntities;
static Era s_currentEraDisplay = Era::StoneAge;
static HWND s_hCityMap = nullptr;
static HWND s_hMapTooltip = nullptr; // Tooltip for the map/HUD

static int s_selectedBuildingIdx = -1; // -1 means none selected
static int s_housingCap = 100;

// --- Tooltip Helper & Globals ---
static HWND s_hTooltip = nullptr;
static WNDPROC s_oldListProc = nullptr;
static Civilization* s_activeCiv = nullptr;
static int s_lastTooltipItem = -1;

static std::wstring ToWStrStatic(const std::string& str) {
    if (str.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstr(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size);
    return wstr;
}

static LRESULT CALLBACK ListBoxSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_MOUSEMOVE) {
        if (s_hTooltip && s_activeCiv) {
            // Relay for tooltip timing
            MSG relayMsg = { hwnd, msg, wParam, lParam };
            SendMessageW(s_hTooltip, TTM_RELAYEVENT, 0, (LPARAM)&relayMsg);

            POINT pt = {LOWORD(lParam), HIWORD(lParam)};
            LRESULT result = SendMessageW(hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));
            
            if (HIWORD(result) == 0) { // Inside client area
                int index = LOWORD(result);
                if (index != s_lastTooltipItem) {
                    s_lastTooltipItem = index;
                    
                    std::wstring text = L"";
                    LPARAM itemData = SendMessageW(hwnd, LB_GETITEMDATA, index, 0);
                    
                    // itemData is index in available vector, or -1
                    if (itemData != -1) {
                        auto available = s_activeCiv->getTech().getAvailableTechs();
                        if (itemData >= 0 && itemData < (LPARAM)available.size()) {
                            text = ToWStrStatic(available[itemData]->description);
                        }
                    }

                    TOOLINFOW ti = { 0 };
                    ti.cbSize = sizeof(TOOLINFOW);
                    ti.hwnd = GetParent(hwnd);
                    ti.uId = (UINT_PTR)hwnd;
                    ti.lpszText = const_cast<LPWSTR>(text.c_str());
                    
                    SendMessageW(s_hTooltip, TTM_UPDATETIPTEXTW, 0, (LPARAM)&ti);
                    
                    if (text.empty()) {
                        SendMessageW(s_hTooltip, TTM_ACTIVATE, FALSE, 0);
                    } else {
                        SendMessageW(s_hTooltip, TTM_ACTIVATE, TRUE, 0);
                    }
                }
            } else {
                s_lastTooltipItem = -1;
                SendMessageW(s_hTooltip, TTM_ACTIVATE, FALSE, 0);
            }
        }
    }
    return CallWindowProcW(s_oldListProc, hwnd, msg, wParam, lParam);
}

static void InitCityMap() {
    s_cityEntities.clear();
    s_selectedBuildingIdx = -1;
    // Start with one Town Hall in the center
    CityEntity townHall;
    townHall.x = 570.0f; // Center of 1140 width
    townHall.y = 375.0f; // Center of 750 height
    townHall.dx = 0; townHall.dy = 0;
    townHall.type = 2; // Town Hall
    townHall.variant = 0;
    townHall.buildingDefIdx = -1;
    townHall.color = C7_GOLD;
    s_cityEntities.push_back(townHall);
}

static bool IsOverlapping(float x, float y, float r) {
    for(const auto& e : s_cityEntities) {
        if(e.type == 0) continue; // Ignore settlers
        float distSq = (e.x - x)*(e.x - x) + (e.y - y)*(e.y - y);
        float minDist = (r + 12.0f); 
        if(distSq < minDist*minDist) return true;
    }
    return false;
}

static void UpdateCityMap(const Civilization& civ) {
    s_currentEraDisplay = civ.getCurrentEra();
    
    // Only update settlers count, buildings are persistent now
    double pop = (double)civ.getPopulation();
    size_t targetSettlers = (size_t)(std::log(pop + 1.0) * 15.0);
    if (targetSettlers > 150) targetSettlers = 150;

    // Count current
    size_t settlers = 0;
    std::vector<int> buildingIndices;

    for(size_t i = 0; i < s_cityEntities.size(); ++i) {
        const auto& e = s_cityEntities[i];
        if(e.type == 0) settlers++;
        else {
            buildingIndices.push_back((int)i);
        }
    }

    // Add Settlers
    while(settlers < targetSettlers) {
        CityEntity s;
        float sx = 570.0f, sy = 375.0f;
        if(!buildingIndices.empty()) {
            const auto& b = s_cityEntities[buildingIndices[rand() % buildingIndices.size()]];
            sx = b.x; sy = b.y;
        }
        s.x = sx; s.y = sy;
        s.dx = (float)(rand() % 20 - 10) / 20.0f;
        s.dy = (float)(rand() % 20 - 10) / 20.0f;
        s.type = 0;
        s.variant = 0;
        s.buildingDefIdx = -1;
        s.color = RGB(240, 240, 240);
        s_cityEntities.push_back(s);
        settlers++;
    }
}

static int s_lastMapTooltipIdx = -2; // -2: init, -1: none

static LRESULT CALLBACK CityMapWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
    case WM_TIMER: {
        RECT rc;
        GetClientRect(hwnd, &rc);
        for(auto& e : s_cityEntities) {
            if(e.type == 0) { // Move settlers
                e.x += e.dx;
                e.y += e.dy;
                if(e.x < 0 || e.x > rc.right) e.dx *= -1;
                if(e.y < 0 || e.y > rc.bottom) e.dy *= -1;
                if(rand()%100 == 0) { e.dx = (float)(rand()%20-10)/20.0f; e.dy = (float)(rand()%20-10)/20.0f; }
            }
        }
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        
        HDC hMemDC = CreateCompatibleDC(hdc);
        HBITMAP hBm = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
        HBITMAP hOldBm = (HBITMAP)SelectObject(hMemDC, hBm);
        
        // Background based on Era
        COLORREF bgCol = RGB(34, 139, 34); // Stone Age Green
        if (s_currentEraDisplay >= Era::Industrial) bgCol = RGB(80, 80, 90); // Industrial Grey
        if (s_currentEraDisplay == Era::Space) bgCol = RGB(20, 20, 40); // Space Dark
        
        HBRUSH hBg = CreateSolidBrush(bgCol);
        FillRect(hMemDC, &rc, hBg);
        DeleteObject(hBg);

        // Draw HUD Background (Bottom 40px)
        RECT rcHud = {0, rc.bottom - 40, rc.right, rc.bottom};
        HBRUSH hHudBg = CreateSolidBrush(RGB(40, 40, 50));
        FillRect(hMemDC, &rcHud, hHudBg);
        DeleteObject(hHudBg);

        // Draw Entities
        for(const auto& e : s_cityEntities) {
            if(e.type == 0) { // Settler
                HBRUSH hBr = CreateSolidBrush(e.color);
                HBRUSH hOld = (HBRUSH)SelectObject(hMemDC, hBr);
                Ellipse(hMemDC, (int)e.x-2, (int)e.y-2, (int)e.x+2, (int)e.y+2);
                SelectObject(hMemDC, hOld);
                DeleteObject(hBr);
            } else { // Building
                int size = (e.type == 2) ? 8 : 5;
                RECT rB = {(int)e.x-size, (int)e.y-size, (int)e.x+size, (int)e.y+size};
                HBRUSH hB = CreateSolidBrush(e.color);
                FillRect(hMemDC, &rB, hB);
                DeleteObject(hB);
                
                // Roof
                POINT pts[3] = {
                    {(int)e.x, (int)e.y-size-4},
                    {(int)e.x-size, (int)e.y-size},
                    {(int)e.x+size, (int)e.y-size}
                };
                HBRUSH hRoof = CreateSolidBrush(RGB(139, 69, 19));
                HBRUSH hOld = (HBRUSH)SelectObject(hMemDC, hRoof);
                HPEN hPen = CreatePen(PS_SOLID, 1, RGB(100, 50, 10));
                HPEN hOldPen = (HPEN)SelectObject(hMemDC, hPen);
                Polygon(hMemDC, pts, 3);
                SelectObject(hMemDC, hOld);
                SelectObject(hMemDC, hOldPen);
                DeleteObject(hRoof);
                DeleteObject(hPen);
            }
        }
        
        // Draw HUD Items
        int xPos = 10;
        int yPos = rc.bottom - 35;
        SetBkMode(hMemDC, TRANSPARENT);
        
        for (int i = 0; i < (int)s_buildingDefs.size(); ++i) {
            const auto& def = s_buildingDefs[i];
            if (s_currentEraDisplay < def.minEra) continue;

            RECT rItem = {xPos, yPos, xPos + 80, yPos + 30};
            
            // Highlight selection
            if (i == s_selectedBuildingIdx) {
                HBRUSH hSel = CreateSolidBrush(RGB(80, 80, 100));
                FillRect(hMemDC, &rItem, hSel);
                DeleteObject(hSel);
                FrameRect(hMemDC, &rItem, (HBRUSH)GetStockObject(WHITE_BRUSH));
            } else {
                HBRUSH hBtn = CreateSolidBrush(RGB(60, 60, 70));
                FillRect(hMemDC, &rItem, hBtn);
                DeleteObject(hBtn);
            }

            // Color indicator
            RECT rCol = {xPos + 5, yPos + 5, xPos + 25, yPos + 25};
            HBRUSH hCol = CreateSolidBrush(def.color);
            FillRect(hMemDC, &rCol, hCol);
            DeleteObject(hCol);

            // Name
            RECT rText = {xPos + 30, yPos, xPos + 80, yPos + 30};
            SetTextColor(hMemDC, RGB(255, 255, 255));
            DrawTextW(hMemDC, def.name.c_str(), -1, &rText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

            xPos += 90;
        }

        BitBlt(hdc, 0, 0, rc.right, rc.bottom, hMemDC, 0, 0, SRCCOPY);
        SelectObject(hMemDC, hOldBm); DeleteObject(hBm); DeleteDC(hMemDC);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_MOUSEMOVE: {
        if (s_hMapTooltip) {
            MSG relayMsg = { hwnd, msg, wParam, lParam };
            SendMessageW(s_hMapTooltip, TTM_RELAYEVENT, 0, (LPARAM)&relayMsg);
        }

        int xPos = LOWORD(lParam);
        int yPos = HIWORD(lParam);
        RECT rc;
        GetClientRect(hwnd, &rc);

        // Check HUD hover
        int hoveredIdx = -1;
        if (yPos > rc.bottom - 50) {
            int hudX = 10;
            for (int i = 0; i < (int)s_buildingDefs.size(); ++i) {
                if (s_currentEraDisplay < s_buildingDefs[i].minEra) continue;
                if (xPos >= hudX && xPos <= hudX + 100) {
                    hoveredIdx = i;
                    break;
                }
                hudX += 110;
            }
        }

        if (hoveredIdx != s_lastMapTooltipIdx) {
            s_lastMapTooltipIdx = hoveredIdx;
            std::wstring text = L"";
            if (hoveredIdx >= 0) {
                const auto& def = s_buildingDefs[hoveredIdx];
                text = def.name + L"\nЦена: $" + std::to_wstring((int)def.costMoney) + 
                       L", Мат: " + std::to_wstring((int)def.costMaterials) + L"\n";
                if(def.popCapAdd > 0) text += L"Жилье: +" + std::to_wstring(def.popCapAdd) + L"\n";
                if(def.foodAdd > 0) text += L"Еда: +" + std::to_wstring((int)def.foodAdd) + L"/ход\n";
                if(def.moneyAdd > 0) text += L"Деньги: +" + std::to_wstring((int)def.moneyAdd) + L"/ход\n";
                if(def.materialsAdd > 0) text += L"Материалы: +" + std::to_wstring((int)def.materialsAdd) + L"/ход\n";
            }

            TOOLINFOW ti = { 0 };
            ti.cbSize = sizeof(TOOLINFOW);
            ti.hwnd = hwnd;
            ti.uId = 1;
            ti.lpszText = const_cast<LPWSTR>(text.c_str());
            SendMessageW(s_hMapTooltip, TTM_UPDATETIPTEXTW, 0, (LPARAM)&ti);
            SendMessageW(s_hMapTooltip, TTM_ACTIVATE, !text.empty(), 0);
        }
        return 0;
    }
    case WM_LBUTTONDOWN: {
        int xPos = LOWORD(lParam);
        int yPos = HIWORD(lParam);
        RECT rc;
        GetClientRect(hwnd, &rc);

        // Check HUD click
        if (yPos > rc.bottom - 40) {
            int hudX = 10;
            for (int i = 0; i < (int)s_buildingDefs.size(); ++i) {
                if (s_currentEraDisplay < s_buildingDefs[i].minEra) continue;
                if (xPos >= hudX && xPos <= hudX + 80) {
                    s_selectedBuildingIdx = i;
                    InvalidateRect(hwnd, nullptr, FALSE);
                    return 0;
                }
                hudX += 90;
            }
            return 0;
        }

        // Place Building
        if (s_selectedBuildingIdx != -1) {
            if (!s_activeCiv) {
                MessageBoxW(hwnd, L"Сначала начните новую игру (Нажмите 'Следующий ход')!", L"Ошибка", MB_OK | MB_ICONWARNING);
                return 0;
            }

            const auto& def = s_buildingDefs[s_selectedBuildingIdx];
            double money = s_activeCiv->getResources().getResource(ResourceType::Money);
            double mats = s_activeCiv->getResources().getResource(ResourceType::Materials);

            if (money >= def.costMoney && mats >= def.costMaterials) {
                if (!IsOverlapping((float)xPos, (float)yPos, 10.0f)) {
                    s_activeCiv->getResources().removeResource(ResourceType::Money, def.costMoney);
                    s_activeCiv->getResources().removeResource(ResourceType::Materials, def.costMaterials);

                    CityEntity b;
                    b.x = (float)xPos; b.y = (float)yPos;
                    b.dx = 0; b.dy = 0;
                    b.type = 1; // Building
                    b.variant = 0;
                    b.buildingDefIdx = s_selectedBuildingIdx;
                    b.color = def.color;
                    s_cityEntities.push_back(b);
                    
                    InvalidateRect(hwnd, nullptr, FALSE);
                } else {
                    MessageBoxW(hwnd, L"Слишком близко к другому зданию!", L"Ошибка", MB_OK | MB_ICONWARNING);
                }
            } else {
                MessageBoxW(hwnd, L"Недостаточно ресурсов!", L"Ошибка", MB_OK | MB_ICONWARNING);
            }
            return 0;
        }

        // Iterate backwards to check topmost entities first
        for (auto it = s_cityEntities.rbegin(); it != s_cityEntities.rend(); ++it) {
            const auto& e = *it;
            if (e.type != 0) { // It's a building
                RECT rB = {(int)e.x-8, (int)e.y-8, (int)e.x+8, (int)e.y+8};
                POINT pt = {xPos, yPos};
                if (PtInRect(&rB, pt)) {
                    std::wstring info;
                    if (e.type == 1 && e.buildingDefIdx >= 0) {
                        info = s_buildingDefs[e.buildingDefIdx].name;
                    } else if (e.type == 2) {
                        info = L"Ратуша.\n\nЦентр вашего поселения.";
                    }
                    MessageBoxW(hwnd, info.c_str(), L"Информация о здании", MB_OK | MB_ICONINFORMATION);
                    return 0; // Stop after finding the first building
                }
            }
        }
        return 0;
    }
    case WM_RBUTTONDOWN: {
        if (s_selectedBuildingIdx != -1) {
            s_selectedBuildingIdx = -1;
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        return 0;
    }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// --- Difficulty Dialog Helper ---
static Difficulty s_selectedDifficulty = Difficulty::Normal;
static bool s_dialogDone = false;

static LRESULT CALLBACK DiffWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN: {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, C7_TEXT);
        SetBkMode(hdc, TRANSPARENT);
        return (LRESULT)CreateSolidBrush(C7_PANEL); // Note: Leaks, but simple for a short-lived dialog
    }
    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED) {
            int id = LOWORD(wParam);
            if (id >= 100 && id <= 103) {
                s_selectedDifficulty = static_cast<Difficulty>(id - 100);
                s_dialogDone = true;
                DestroyWindow(hwnd);
            }
        }
        break;
    case WM_CLOSE:
        s_dialogDone = true;
        DestroyWindow(hwnd);
        break;
    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

static Difficulty AskDifficulty(HINSTANCE hInstance, HWND hParent) {
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = DiffWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"DiffDialog";
    wc.hbrBackground = CreateSolidBrush(C7_BG);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassExW(&wc);

    RECT rcOwner;
    GetWindowRect(hParent, &rcOwner);
    int width = 250;
    int height = 240;
    int x = rcOwner.left + (rcOwner.right - rcOwner.left - width) / 2;
    int y = rcOwner.top + (rcOwner.bottom - rcOwner.top - height) / 2;

    HWND hDlg = CreateWindowExW(WS_EX_DLGMODALFRAME, L"DiffDialog", L"Выберите сложность",
        WS_VISIBLE | WS_POPUP | WS_CAPTION | WS_SYSMENU,
        x, y, width, height, hParent, nullptr, hInstance, nullptr);

    HFONT hFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                              CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                              VARIABLE_PITCH, L"Segoe UI");

    const wchar_t* labels[] = { L"Лёгкий", L"Нормальный", L"Сложный", L"Кошмар" };
    for (int i = 0; i < 4; ++i) {
        HWND hBtn = CreateWindowW(L"BUTTON", labels[i], WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            25, 20 + i * 45, 200, 35, hDlg, (HMENU)(INT_PTR)(100 + i), hInstance, nullptr);
        SendMessageW(hBtn, WM_SETFONT, (WPARAM)hFont, 0);
    }

    EnableWindow(hParent, FALSE);
    s_dialogDone = false;
    s_selectedDifficulty = Difficulty::Normal;

    MSG msg;
    while (!s_dialogDone && GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    EnableWindow(hParent, TRUE);
    SetForegroundWindow(hParent);
    UnregisterClassW(L"DiffDialog", hInstance);
    DeleteObject(hFont);
    
    return s_selectedDifficulty;
}
// --------------------------------

// --- Game Over Dialog Helper ---
static bool s_gameOverDialogDone = false;

static LRESULT CALLBACK GameOverWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            s_gameOverDialogDone = true;
            DestroyWindow(hwnd);
        }
        break;
    case WM_CLOSE:
        s_gameOverDialogDone = true;
        DestroyWindow(hwnd);
        break;
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN: {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, C7_TEXT);
        SetBkMode(hdc, TRANSPARENT);
        return (LRESULT)CreateSolidBrush(C7_PANEL); // Note: Leaks, but simple for a short-lived dialog
    }
    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

static void ShowGameOverDialog(HINSTANCE hInstance, HWND hParent, GameResult result, const Civilization& civ) {
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = GameOverWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"GameOverDialog";
    wc.hbrBackground = CreateSolidBrush(C7_BG);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassExW(&wc);

    int width = 450;
    int height = 320;
    RECT rcOwner;
    GetWindowRect(hParent, &rcOwner);
    int x = rcOwner.left + (rcOwner.right - rcOwner.left - width) / 2;
    int y = rcOwner.top + (rcOwner.bottom - rcOwner.top - height) / 2;

    HWND hDlg = CreateWindowExW(WS_EX_DLGMODALFRAME, L"GameOverDialog", L"Результаты игры",
        WS_VISIBLE | WS_POPUP | WS_CAPTION | WS_SYSMENU,
        x, y, width, height, hParent, nullptr, hInstance, nullptr);

    HFONT hTitleFont = CreateFontW(32, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Georgia");
    HFONT hTextFont = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
    HFONT hBoldFont = CreateFontW(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");

    bool isVictory = (result == GameResult::VictorySpace || result == GameResult::VictoryEconomy || result == GameResult::VictoryTech);
    std::wstring title = isVictory ? L"ПОБЕДА!" : L"ПОРАЖЕНИЕ";
    
    // Title
    HWND hTitle = CreateWindowW(L"STATIC", title.c_str(), WS_VISIBLE | WS_CHILD | SS_CENTER, 
        10, 20, width - 20, 45, hDlg, nullptr, hInstance, nullptr);
    SendMessageW(hTitle, WM_SETFONT, (WPARAM)hTitleFont, 0);
    
    // Description
    std::wstring desc;
    switch (result) {
        case GameResult::VictorySpace: desc = L"Ваша цивилизация покорила космос!"; break;
        case GameResult::VictoryEconomy: desc = L"Экономическое процветание достигнуто!"; break;
        case GameResult::VictoryTech: desc = L"Технологическое превосходство!"; break;
        case GameResult::DefeatPopulation: desc = L"Ваша цивилизация вымерла..."; break;
        case GameResult::DefeatEcology: desc = L"Экологическая катастрофа!"; break;
        case GameResult::DefeatEconomy: desc = L"Полный экономический крах."; break;
        default: desc = L"Игра завершена."; break;
    }
    
    HWND hDesc = CreateWindowW(L"STATIC", desc.c_str(), WS_VISIBLE | WS_CHILD | SS_CENTER, 
        20, 80, width - 40, 30, hDlg, nullptr, hInstance, nullptr);
    SendMessageW(hDesc, WM_SETFONT, (WPARAM)hTextFont, 0);

    // Stats
    std::wstring stats1 = L"Ходов: " + std::to_wstring(civ.getTurn()) + 
                          L"   Население: " + ToWStrStatic(std::to_string(civ.getPopulation()));
    HWND hStats1 = CreateWindowW(L"STATIC", stats1.c_str(), WS_VISIBLE | WS_CHILD | SS_CENTER, 
        20, 130, width - 40, 25, hDlg, nullptr, hInstance, nullptr);
    SendMessageW(hStats1, WM_SETFONT, (WPARAM)hBoldFont, 0);

    std::wstring stats2 = L"Эпоха: " + ToWStrStatic(eraToString(civ.getCurrentEra()));
    HWND hStats2 = CreateWindowW(L"STATIC", stats2.c_str(), WS_VISIBLE | WS_CHILD | SS_CENTER, 
        20, 160, width - 40, 25, hDlg, nullptr, hInstance, nullptr);
    SendMessageW(hStats2, WM_SETFONT, (WPARAM)hBoldFont, 0);

    // Button
    HWND hBtn = CreateWindowW(L"BUTTON", L"В главное меню", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        (width - 160) / 2, 220, 160, 40, hDlg, (HMENU)(INT_PTR)IDOK, hInstance, nullptr);
    SendMessageW(hBtn, WM_SETFONT, (WPARAM)hTextFont, 0);

    EnableWindow(hParent, FALSE);
    s_gameOverDialogDone = false;
    MSG msg;
    while (!s_gameOverDialogDone && GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    EnableWindow(hParent, TRUE);
    SetForegroundWindow(hParent);
    
    UnregisterClassW(L"GameOverDialog", hInstance);
    DeleteObject(hTitleFont);
    DeleteObject(hTextFont);
    DeleteObject(hBoldFont);
}
// --------------------------------

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
    if (size == 0) return L"";
    std::wstring wstr(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size);
    if (size > 0 && wstr[size - 1] == L'\0') wstr.resize(size - 1); // Убираем лишний null-терминатор
    return wstr;
}

int Win32Gui::run(HINSTANCE hInstance, int nCmdShow) {
    m_hInstance = hInstance;
    std::srand((unsigned int)std::time(nullptr));

    INITCOMMONCONTROLSEX icex;
    icex.dwICC = ICC_PROGRESS_CLASS | ICC_LISTVIEW_CLASSES | ICC_UPDOWN_CLASS;
    InitCommonControlsEx(&icex);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = StaticWndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_HAND);
    wc.hbrBackground = CreateSolidBrush(C7_BG);
    wc.lpszClassName = CLASS_NAME;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

    if (!RegisterClassExW(&wc)) {
        return 1;
    }
    
    // Register CityMap Class
    WNDCLASSEXW wcMap = {0};
    wcMap.cbSize = sizeof(WNDCLASSEXW);
    wcMap.lpfnWndProc = CityMapWndProc;
    wcMap.hInstance = hInstance;
    wcMap.lpszClassName = L"CityMapClass";
    RegisterClassExW(&wcMap);

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
    ShowWindow(m_mainWindow, SW_MAXIMIZE); // Force maximized window
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
    // Resize window to fit new layout
    SetWindowPos(m_mainWindow, nullptr, 0, 0, 1920, 1080, SWP_NOZORDER);

    // Create brushes
    m_bgBrush = CreateSolidBrush(C7_BG);
    m_panelBrush = CreateSolidBrush(C7_PANEL);

    // Fonts
    HFONT hTitleFont = CreateFontW(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                    DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                                    CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                    VARIABLE_PITCH, L"Segoe UI");
    HFONT hBtnFont = CreateFontW(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                                   CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                   VARIABLE_PITCH, L"Segoe UI");
    HFONT hLabelFont = CreateFontW(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                                   CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                   VARIABLE_PITCH, L"Georgia");
    HFONT hValueFont = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                    DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                                    CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                    VARIABLE_PITCH, L"Consolas");

    int childStyle = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS;
    int btnStyle = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | BS_PUSHBUTTON;

    int x = 20, y = 20;

    // Title
    HWND hTitle = CreateWindowW(L"STATIC", L"СИМУЛЯТОР ЦИВИЛИЗАЦИИ",
                                 childStyle | SS_LEFT, x, y, 400, 45,
                                 m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hTitle, WM_SETFONT, (WPARAM)hTitleFont, 0);

    // Era and Turn
    y = 70;
    m_eraLabel = CreateWindowW(L"STATIC", L"Каменный век",
                                childStyle | SS_LEFT, x, y, 300, 30,
                                m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_eraLabel, WM_SETFONT, (WPARAM)hLabelFont, 0);

    m_turnLabel = CreateWindowW(L"STATIC", L"Ход: 0",
                                childStyle | SS_LEFT, x + 320, y, 150, 30,
                                m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_turnLabel, WM_SETFONT, (WPARAM)hLabelFont, 0);

    // Stats in one row - compact
    y = 25; // Top right area for stats
    int statX = 500;
    int statGap = 250;

    HWND hPopTitle = CreateWindowW(L"STATIC", L"Население:",
                  childStyle | SS_LEFT, statX, y, 100, 25,
                  m_mainWindow, nullptr, m_hInstance, nullptr);
    
    m_populationLabel = CreateWindowW(L"STATIC", L"1000",
                                       childStyle | SS_LEFT, statX + 110, y, 100, 25,
                                       m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_populationLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    HWND hHappyTitle = CreateWindowW(L"STATIC", L"Счастье:",
                  childStyle | SS_LEFT, statX + statGap, y, 90, 25,
                  m_mainWindow, nullptr, m_hInstance, nullptr);
    m_happinessLabel = CreateWindowW(L"STATIC", L"70%",
                                     childStyle | SS_LEFT, statX + statGap + 100, y, 80, 25,
                                     m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_happinessLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    HWND hEcoTitle = CreateWindowW(L"STATIC", L"Экология:",
                  childStyle | SS_LEFT, statX + statGap*2, y, 90, 25,
                  m_mainWindow, nullptr, m_hInstance, nullptr);
    m_ecologyLabel = CreateWindowW(L"STATIC", L"90%",
                                    childStyle | SS_LEFT, statX + statGap*2 + 100, y, 80, 25,
                                    m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_ecologyLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    HWND hMilTitle = CreateWindowW(L"STATIC", L"Армия:",
                  childStyle | SS_LEFT, statX + statGap*3, y, 70, 25,
                  m_mainWindow, nullptr, m_hInstance, nullptr);
    m_militaryLabel = CreateWindowW(L"STATIC", L"10",
                                    childStyle | SS_LEFT, statX + statGap*3 + 80, y, 80, 25,
                                    m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_militaryLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    HWND hTechTitle = CreateWindowW(L"STATIC", L"Технологии:",
                  childStyle | SS_LEFT, statX + statGap*4, y, 110, 25,
                  m_mainWindow, nullptr, m_hInstance, nullptr);
    m_techLabel = CreateWindowW(L"STATIC", L"0/100",
                                childStyle | SS_LEFT, statX + statGap*4 + 120, y, 80, 25,
                                m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_techLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    // Resources row
    y = 70; // Second row
    HWND hFoodTitle = CreateWindowW(L"STATIC", L"Еда:",
                  childStyle | SS_LEFT, statX, y, 60, 25,
                  m_mainWindow, nullptr, m_hInstance, nullptr);
    m_foodLabel = CreateWindowW(L"STATIC", L"100",
                                 childStyle | SS_LEFT, statX + 70, y, 100, 25,
                                 m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_foodLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    HWND hMoneyTitle = CreateWindowW(L"STATIC", L"Деньги:",
                  childStyle | SS_LEFT, statX + statGap, y, 80, 25,
                  m_mainWindow, nullptr, m_hInstance, nullptr);
    m_moneyLabel = CreateWindowW(L"STATIC", L"500",
                                 childStyle | SS_LEFT, statX + statGap + 90, y, 100, 25,
                                 m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_moneyLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    HWND hEnergyTitle = CreateWindowW(L"STATIC", L"Энергия:",
                  childStyle | SS_LEFT, statX + statGap*2, y, 90, 25,
                  m_mainWindow, nullptr, m_hInstance, nullptr);
    m_energyLabel = CreateWindowW(L"STATIC", L"50",
                                   childStyle | SS_LEFT, statX + statGap*2 + 100, y, 100, 25,
                                   m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_energyLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    HWND hMatTitle = CreateWindowW(L"STATIC", L"Материалы:",
                  childStyle | SS_LEFT, statX + statGap*3, y, 110, 25,
                  m_mainWindow, nullptr, m_hInstance, nullptr);
    m_materialsLabel = CreateWindowW(L"STATIC", L"100",
                                       childStyle | SS_LEFT, statX + statGap*3 + 120, y, 100, 25,
                                       m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(m_materialsLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    // --- CITY MAP VISUALIZER ---
    // Center Column
    int mapX = 390;
    int mapY = 160;
    int mapW = 1140;
    int mapH = 750;

    HWND hMapTitle = CreateWindowW(L"STATIC", L"Вид поселения:",
                  childStyle | SS_LEFT, mapX, mapY - 30, 200, 25,
                  m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hMapTitle, WM_SETFONT, (WPARAM)hLabelFont, 0);
    
    s_hCityMap = CreateWindowW(L"CityMapClass", nullptr,
                               WS_VISIBLE | WS_CHILD | WS_BORDER,
                               mapX, mapY, mapW, mapH,
                               m_mainWindow, nullptr, m_hInstance, nullptr);
    SetTimer(s_hCityMap, 1, 50, nullptr); // Animation timer

    // Setup Map Tooltip
    s_hMapTooltip = CreateWindowExW(0, TOOLTIPS_CLASSW, nullptr,
                                    WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
                                    CW_USEDEFAULT, CW_USEDEFAULT,
                                    CW_USEDEFAULT, CW_USEDEFAULT,
                                    s_hCityMap, nullptr, m_hInstance, nullptr);
    TOOLINFOW tiMap = { 0 };
    tiMap.cbSize = sizeof(TOOLINFOW);
    tiMap.uFlags = TTF_IDISHWND | TTF_TRANSPARENT | TTF_SUBCLASS;
    tiMap.hwnd = s_hCityMap;
    tiMap.uId = 1;
    tiMap.lpszText = (LPWSTR)L"";
    SendMessageW(s_hMapTooltip, TTM_ADDTOOLW, 0, (LPARAM)&tiMap);

    // Technologies List
    // Left Column
    int techX = 20;
    int techY = 160;
    int techW = 350;
    int techH = 750;

    HWND hTechListTitle = CreateWindowW(L"STATIC", L"Доступные технологии:",
                  childStyle | SS_LEFT, techX, techY - 30, 280, 25,
                  m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hTechListTitle, WM_SETFONT, (WPARAM)hLabelFont, 0);
    
    m_techList = CreateWindowW(WC_LISTBOXW, nullptr,
                                childStyle | WS_BORDER | LBS_NOTIFY | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS,
                                techX, techY, techW, techH,
                                m_mainWindow, (HMENU)(INT_PTR)IDC_LIST_TECHS,
                                m_hInstance, nullptr);
    SendMessageW(m_techList, WM_SETFONT, (WPARAM)hValueFont, 0);

    // Setup Tooltip
    s_hTooltip = CreateWindowExW(0, TOOLTIPS_CLASSW, nullptr,
                                WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                m_mainWindow, nullptr, m_hInstance, nullptr);
    
    TOOLINFOW ti = { 0 };
    ti.cbSize = sizeof(TOOLINFOW);
    ti.uFlags = TTF_IDISHWND | TTF_TRANSPARENT;
    ti.hwnd = m_mainWindow;
    ti.uId = (UINT_PTR)m_techList;
    ti.lpszText = (LPWSTR)L"";
    SendMessageW(s_hTooltip, TTM_ADDTOOLW, 0, (LPARAM)&ti);

    // Subclass ListBox
    s_oldListProc = (WNDPROC)SetWindowLongPtrW(m_techList, GWLP_WNDPROC, (LONG_PTR)ListBoxSubclassProc);

    // Events panel (right side)
    int evtX = 1550;
    int evtY = 160;
    int evtW = 350;
    int evtH = 750;

    HWND hEventTitle = CreateWindowW(L"STATIC", L"События:",
                  childStyle | SS_LEFT, evtX, evtY - 30, 180, 25,
                  m_mainWindow, nullptr, m_hInstance, nullptr);
    SendMessageW(hEventTitle, WM_SETFONT, (WPARAM)hLabelFont, 0);

    m_eventLabel = CreateWindowW(L"EDIT", L"Нажмите 'Следующий ход' для начала игры!",
                                  childStyle | ES_MULTILINE | ES_READONLY | WS_VSCROLL,
                                  evtX, evtY, evtW, evtH,
                                  m_mainWindow, nullptr,
                                  m_hInstance, nullptr);
    SendMessageW(m_eventLabel, WM_SETFONT, (WPARAM)hValueFont, 0);

    // Action Buttons row
    int btnY = 950;
    int btnX = 390;

    m_nextTurnBtn = CreateWindowW(L"BUTTON", L"Следующий ход",
                                   btnStyle, btnX, btnY, 200, 60,
                                   m_mainWindow, (HMENU)(INT_PTR)IDC_BTN_NEXT,
                                   m_hInstance, nullptr);
    SendMessageW(m_nextTurnBtn, WM_SETFONT, (WPARAM)hBtnFont, 0);
    btnX += 220;

    m_investBtn = CreateWindowW(L"BUTTON", L"Инвестировать",
                                 btnStyle, btnX, btnY, 180, 60,
                                 m_mainWindow, (HMENU)(INT_PTR)IDC_BTN_INVEST,
                                 m_hInstance, nullptr);
    SendMessageW(m_investBtn, WM_SETFONT, (WPARAM)hBtnFont, 0);
    btnX += 200;

    m_researchBtn = CreateWindowW(L"BUTTON", L"Исследовать",
                                   btnStyle, btnX, btnY, 180, 60,
                                   m_mainWindow, (HMENU)(INT_PTR)IDC_BTN_RESEARCH,
                                   m_hInstance, nullptr);
    SendMessageW(m_researchBtn, WM_SETFONT, (WPARAM)hBtnFont, 0);
    
    // System buttons on the right
    btnX = 1550;

    HFONT hSmallBtnFont = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
    m_saveBtn = CreateWindowW(L"BUTTON", L"Сохранить",
                               btnStyle, btnX, btnY, 160, 40,
                               m_mainWindow, (HMENU)(INT_PTR)IDC_BTN_SAVE,
                               m_hInstance, nullptr);
    SendMessageW(m_saveBtn, WM_SETFONT, (WPARAM)hSmallBtnFont, 0);

    m_loadBtn = CreateWindowW(L"BUTTON", L"Загрузить",
                               btnStyle, btnX + 170, btnY, 160, 40,
                               m_mainWindow, (HMENU)(INT_PTR)IDC_BTN_LOAD,
                               m_hInstance, nullptr);
    SendMessageW(m_loadBtn, WM_SETFONT, (WPARAM)hSmallBtnFont, 0);

    m_helpBtn = CreateWindowW(L"BUTTON", L"Помощь",
                               btnStyle, btnX, btnY + 50, 160, 40,
                               m_mainWindow, (HMENU)(INT_PTR)IDC_BTN_HELP,
                               m_hInstance, nullptr);
    SendMessageW(m_helpBtn, WM_SETFONT, (WPARAM)hSmallBtnFont, 0);

    m_quitBtn = CreateWindowW(L"BUTTON", L"Выход",
                              btnStyle, btnX + 170, btnY + 50, 160, 40,
                              m_mainWindow, (HMENU)(INT_PTR)IDC_BTN_QUIT,
                              m_hInstance, nullptr);
    SendMessageW(m_quitBtn, WM_SETFONT, (WPARAM)hSmallBtnFont, 0);
}

void Win32Gui::updateAllUI() {
    updateStats();
    updateResources();
    updateTechTree();
    updateEra();
    if(m_civ) UpdateCityMap(*m_civ);
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
    s_lastTooltipItem = -1;

    // Показываем общий уровень технологий
    int overallLevel = m_civ->getTech().getOverallTechLevel();
    std::wstringstream ssHeader;
    ssHeader << L"=== Уровень технологий: " << overallLevel << "/100 ===";
    int idx = (int)SendMessageW(m_techList, LB_ADDSTRING, 0, (LPARAM)ssHeader.str().c_str());
    SendMessageW(m_techList, LB_SETITEMDATA, idx, -1); // -1 means not selectable

    // Показываем прогресс по веткам
    for (int i = 0; i < static_cast<int>(TechBranch::COUNT); ++i) {
        auto branch = static_cast<TechBranch>(i);
        int level = m_civ->getTech().getBranchLevel(branch);
        double progress = m_civ->getTech().getBranchProgress(branch);
        double threshold = m_civ->getTech().getBranchThreshold(branch);
        
        std::wstringstream ss;
        ss << L"  " << toWStr(techBranchToString(branch)) << L": Ур." << level;
        idx = (int)SendMessageW(m_techList, LB_ADDSTRING, 0, (LPARAM)ss.str().c_str());
        SendMessageW(m_techList, LB_SETITEMDATA, idx, 0x100 + i);
    }

    // Пустая строка
    idx = (int)SendMessageW(m_techList, LB_ADDSTRING, 0, (LPARAM)L"");
    SendMessageW(m_techList, LB_SETITEMDATA, idx, -1);

    // Исследованные технологии
    idx = (int)SendMessageW(m_techList, LB_ADDSTRING, 0, (LPARAM)L"--- Исследовано ---");
    SendMessageW(m_techList, LB_SETITEMDATA, idx, -1);

    auto researched = m_civ->getTech().getResearchedTechs();
    if (researched.empty()) {
        idx = (int)SendMessageW(m_techList, LB_ADDSTRING, 0, (LPARAM)L"  (пока нет)");
        SendMessageW(m_techList, LB_SETITEMDATA, idx, -1);
    } else {
        for (const auto* tech : researched) {
            std::wstring text = L"  [✓] " + toWStr(tech->name) + L" (Ур." + std::to_wstring(tech->level) + L")";
            idx = (int)SendMessageW(m_techList, LB_ADDSTRING, 0, (LPARAM)text.c_str());
            SendMessageW(m_techList, LB_SETITEMDATA, idx, -1);
        }
    }

    // Пустая строка
    idx = (int)SendMessageW(m_techList, LB_ADDSTRING, 0, (LPARAM)L"");
    SendMessageW(m_techList, LB_SETITEMDATA, idx, -1);

    // Доступные технологии
    idx = (int)SendMessageW(m_techList, LB_ADDSTRING, 0, (LPARAM)L"--- Доступные ---");
    SendMessageW(m_techList, LB_SETITEMDATA, idx, -1);

    auto available = m_civ->getTech().getAvailableTechs();
    if (available.empty()) {
        idx = (int)SendMessageW(m_techList, LB_ADDSTRING, 0, (LPARAM)L"  (доступных нет)");
        SendMessageW(m_techList, LB_SETITEMDATA, idx, -1);
    } else {
        for (size_t i = 0; i < available.size(); ++i) {
            const auto* tech = available[i];
            std::wstringstream ss;
            ss << L"  [ ] " << toWStr(tech->name) << L" (Ур." << tech->level << L", $" << tech->cost << L")";
            idx = (int)SendMessageW(m_techList, LB_ADDSTRING, 0, (LPARAM)ss.str().c_str());
            SendMessageW(m_techList, LB_SETITEMDATA, idx, (LPARAM)i); // Store index in available vector
        }
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
    size_t start = (history.size() > 10) ? history.size() - 10 : 0;
    for (size_t i = start; i < history.size(); ++i) {
        const auto& e = history[i];
        ss << L"[" << toWStr(eventTypeToString(e.type)) << L"] " << toWStr(e.name) << L"\r\n";
    }
    SetWindowTextW(m_eventLabel, ss.str().c_str());
}

void Win32Gui::updateEra() {
    if (!m_civ) return;
    SetWindowTextW(m_eraLabel, toWStr(eraToString(m_civ->getCurrentEra())).c_str());
}

static void ApplyCityBonuses(Civilization* civ) {
    if (!civ) return;
    
    int housing = 50; // Base housing from Town Hall
    double food = 0, money = 0, mat = 0, energy = 0, mil = 0;
    
    for (const auto& e : s_cityEntities) {
        if (e.type == 1 && e.buildingDefIdx >= 0) {
            const auto& def = s_buildingDefs[e.buildingDefIdx];
            housing += def.popCapAdd;
            food += def.foodAdd;
            money += def.moneyAdd;
            mat += def.materialsAdd;
            energy += def.energyAdd;
            mil += def.militaryAdd;
        }
    }
    
    civ->getResources().addResource(ResourceType::Food, food);
    civ->getResources().addResource(ResourceType::Money, money);
    civ->getResources().addResource(ResourceType::Materials, mat);
    civ->getResources().addResource(ResourceType::Energy, energy);
    civ->modifyMilitary(mil);
    
    if (civ->getPopulation() > housing) {
        civ->modifyPopulation(housing - civ->getPopulation());
    }
}

void Win32Gui::onNextTurn() {
    if (!m_civ) {
        m_difficulty = AskDifficulty(m_hInstance, m_mainWindow);
        m_civ = std::make_unique<Civilization>(u8"Цивилизация");
        m_events = std::make_unique<EventSystem>();
        m_saveSystem = std::make_unique<SaveSystem>();
        m_events->init(m_difficulty);
        s_activeCiv = m_civ.get();
        updateEra();
        InitCityMap();
        updateResources();
    }

    GameEvent event = m_events->generateEvent(m_civ->getCurrentEra(), m_civ->getTurn());
    m_events->recordEvent(event);
    m_civ->applyEvent(event);
    m_civ->processTurn();

    ApplyCityBonuses(m_civ.get());
    UpdateCityMap(*m_civ);
    updateAllUI(); // Update stats first, so we don't overwrite event text later

    // Update events display with full info (like console version)
    std::wstringstream ss;
    ss << L"=== " << toWStr(eventTypeToString(event.type)) << L": " << toWStr(event.name) << L" ===\r\n\r\n";
    ss << toWStr(event.description) << L"\r\n\r\n";
    
    ss << L"Влияние на поселение:\r\n";

    bool hasEffects = false;

    // Население
    if (event.populationMultiplier != 1.0) {
        double pct = (event.populationMultiplier - 1.0) * 100.0;
        if (pct < 0) ss << L"🔴 Население сократилось на " << std::abs((int)pct) << L"%\r\n";
        else ss << L"🟢 Население выросло на " << (int)pct << L"%\r\n";
        hasEffects = true;
    }
    if (event.populationEffect != 0) {
        if (event.populationEffect < 0) ss << L"🔴 Потеряно " << std::abs(event.populationEffect) << L" жителей\r\n";
        else ss << L"🟢 Прибыло " << event.populationEffect << L" новых жителей\r\n";
        hasEffects = true;
    }

    // Характеристики
    if (event.happinessEffect != 0) {
        if (event.happinessEffect < 0) ss << L"🔴 Счастье упало на " << std::abs((int)event.happinessEffect) << L"%\r\n";
        else ss << L"🟢 Счастье выросло на " << (int)event.happinessEffect << L"%\r\n";
        hasEffects = true;
    }
    if (event.ecologyEffect != 0) {
        if (event.ecologyEffect < 0) ss << L"🔴 Экология ухудшилась на " << std::abs((int)event.ecologyEffect) << L"%\r\n";
        else ss << L"🟢 Экология улучшилась на " << (int)event.ecologyEffect << L"%\r\n";
        hasEffects = true;
    }
    if (event.militaryEffect != 0) {
        if (event.militaryEffect < 0) ss << L"🔴 Военная мощь снизилась на " << std::abs((int)event.militaryEffect) << L"\r\n";
        else ss << L"🟢 Военная мощь возросла на " << (int)event.militaryEffect << L"\r\n";
        hasEffects = true;
    }

    // Ресурсы
    if (event.economyEffect != 0) {
        if (event.economyEffect < 0) ss << L"🔴 Потеряно " << std::abs((int)event.economyEffect) << L" ед. денег\r\n";
        else ss << L"🟢 Получено " << (int)event.economyEffect << L" ед. денег\r\n";
        hasEffects = true;
    }
    if (event.foodEffect != 0) {
        if (event.foodEffect < 0) ss << L"🔴 Потеряно " << std::abs((int)event.foodEffect) << L" ед. еды\r\n";
        else ss << L"🟢 Получено " << (int)event.foodEffect << L" ед. еды\r\n";
        hasEffects = true;
    }
    if (event.materialsEffect != 0) {
        if (event.materialsEffect < 0) ss << L"🔴 Потеряно " << std::abs((int)event.materialsEffect) << L" ед. материалов\r\n";
        else ss << L"🟢 Получено " << (int)event.materialsEffect << L" ед. материалов\r\n";
        hasEffects = true;
    }
    if (event.energyEffect != 0) {
        if (event.energyEffect < 0) ss << L"🔴 Потеряно " << std::abs((int)event.energyEffect) << L" ед. энергии\r\n";
        else ss << L"🟢 Получено " << (int)event.energyEffect << L" ед. энергии\r\n";
        hasEffects = true;
    }

    // Технологии
    if (event.techBoost > 0) {
        ss << L"🔵 Научный прорыв! (+" << event.techBoost << L" к прогрессу технологий)\r\n";
        hasEffects = true;
    }
    
    if (!hasEffects) {
        ss << L"(Событие не оказало существенного влияния)\r\n";
    }
    
    ss << L"\r\n=== История событий ===\r\n";
    
    const auto& history = m_events->getEventHistory();
    size_t start = (history.size() > 5) ? history.size() - 5 : 0;
    for (size_t i = start; i < history.size(); ++i) {
        const auto& e = history[i];
        ss << L"- [" << toWStr(eventTypeToString(e.type)) << L"] " << toWStr(e.name) << L"\r\n";
    }
    SetWindowTextW(m_eventLabel, ss.str().c_str());
    // Scroll to top to see the new event
    SendMessageW(m_eventLabel, EM_SETSEL, 0, 0);
    SendMessageW(m_eventLabel, EM_SCROLLCARET, 0, 0);

    // Check Game Over
    GameResult result = m_civ->checkGameResult();
    if (result == GameResult::InProgress && m_civ->getCurrentEra() == Era::Space) {
        result = GameResult::VictorySpace;
    }
    if (result != GameResult::InProgress) {
        ShowGameOverDialog(m_hInstance, m_mainWindow, result, *m_civ);
        
        // Reset game
        m_civ.reset();
        m_events.reset();
        m_saveSystem.reset();
        s_activeCiv = nullptr;
        
        updateAllUI();
        SetWindowTextW(m_eventLabel, L"Игра окончена. Нажмите 'Следующий ход' для новой игры.");
    }
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
    
    int itemData = (int)SendMessageW(m_techList, LB_GETITEMDATA, sel, 0);
    if (itemData == -1) {
        MessageBoxW(m_mainWindow, L"Выберите доступную технологию или ветку для инвестиции!", L"Ошибка", MB_OK | MB_ICONWARNING);
        return;
    }
    
    TechBranch branch;
    // Если выбрана ветка (индекс >= 0x100)
    if (itemData >= 0x100) {
        int branchIdx = itemData - 0x100;
        if (branchIdx >= 0 && branchIdx < (int)TechBranch::COUNT) {
            branch = static_cast<TechBranch>(branchIdx);
        } else {
            return;
        }
    } else {
        // Если выбрана конкретная технология
        auto available = m_civ->getTech().getAvailableTechs();
        if (itemData < 0 || itemData >= (int)available.size()) return;
        branch = available[itemData]->branch;
    }

    double money = m_civ->getResources().getResource(ResourceType::Money);
    
    // Invest up to 100, or whatever is available
    double amount = 100.0;
    if (money < amount) amount = money;

    if (amount <= 0) {
        MessageBoxW(m_mainWindow, L"Нет денег для инвестиций!", L"Ошибка", MB_OK | MB_ICONWARNING);
        return;
    }
    
    m_civ->getResources().addResource(ResourceType::Money, -amount);
    m_civ->getTech().investInBranch(branch, amount);
    
    updateResources();
    updateTechTree(); // Обновляем прогресс-бары
    
    std::wstring msg = L"Инвестировано $" + std::to_wstring((int)amount) + L" в ветку " + toWStr(techBranchToString(branch));
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
    
    int itemData = (int)SendMessageW(m_techList, LB_GETITEMDATA, sel, 0);
    
    if (itemData >= 0x100) {
        MessageBoxW(m_mainWindow, L"Выберите конкретную технологию для исследования, а не ветку!", L"Ошибка", MB_OK | MB_ICONWARNING);
        return;
    }
    
    if (itemData == -1) {
        MessageBoxW(m_mainWindow, L"Выберите доступную технологию из списка!", L"Ошибка", MB_OK | MB_ICONWARNING);
        return;
    }
    
    auto available = m_civ->getTech().getAvailableTechs();
    if (itemData < 0 || itemData >= (int)available.size()) return;
    const auto* tech = available[itemData];
    double money = m_civ->getResources().getResource(ResourceType::Money);
    
    if (money < tech->cost) {
        MessageBoxW(m_mainWindow, L"Недостаточно денег для исследования!", L"Ошибка", MB_OK | MB_ICONWARNING);
        return;
    }
    
    m_civ->getResources().addResource(ResourceType::Money, -tech->cost);
    m_civ->getTech().researchTech(tech->name);
    
    updateTechTree();
    updateResources();
    
    if (m_civ->checkGameResult() == GameResult::VictorySpace) {
        MessageBoxW(m_mainWindow, L"ПОЗДРАВЛЯЕМ! Вы достигли Космической эры!", 
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
        s_activeCiv = m_civ.get();
        updateEvents(); // Explicitly update events log on load
        UpdateCityMap(*m_civ);
    }
}

void Win32Gui::onHelpBtn() {
    MessageBoxW(m_mainWindow, 
        L"СИМУЛЯТОР ЦИВИЛИЗАЦИИ\n\n"
        L"ЦЕЛЬ: Доведите цивилизацию до Космической эры!\n\n"
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
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
            if (pDIS->CtlID == IDC_LIST_TECHS && pDIS->itemID != (UINT)-1) {
                HDC hdc = pDIS->hDC;
                RECT rc = pDIS->rcItem;
                int itemData = (int)pDIS->itemData;

                // Background
                if (pDIS->itemState & ODS_SELECTED) {
                    HBRUSH hSelectionBrush = CreateSolidBrush(C7_SELECTION);
                    FillRect(hdc, &rc, hSelectionBrush);
                    DeleteObject(hSelectionBrush);
                    SetTextColor(hdc, C7_TEXT);
                    SetBkColor(hdc, C7_SELECTION);
                } else {
                    FillRect(hdc, &rc, m_panelBrush);
                    SetTextColor(hdc, C7_TEXT);
                    SetBkColor(hdc, C7_PANEL);
                }

                // Draw Branch Progress Bar
                if (itemData >= 0x100 && m_civ) {
                    int branchIdx = itemData - 0x100;
                    if (branchIdx >= 0 && branchIdx < (int)TechBranch::COUNT) {
                        auto branch = static_cast<TechBranch>(branchIdx);                        
                        double progress = m_civ->getTech().getBranchProgress(branch);
                        double threshold = m_civ->getTech().getBranchThreshold(branch);
                        double ratio = (threshold > 0) ? (progress / threshold) : 0.0;
                        if (ratio > 1.0) ratio = 1.0;

                        // Text Label
                        wchar_t buf[256];
                        SendMessageW(pDIS->hwndItem, LB_GETTEXT, pDIS->itemID, (LPARAM)buf);
                        RECT rcText = rc;
                        rcText.right = rc.left + 140;
                        int oldMode = SetBkMode(hdc, TRANSPARENT);
                        DrawTextW(hdc, buf, -1, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

                        // Bar
                        RECT rcBar = rc;
                        rcBar.left = rcText.right;
                        rcBar.right -= 5;
                        rcBar.top += 2;
                        rcBar.bottom -= 2;

                        HBRUSH hBgBar = CreateSolidBrush(C7_SELECTION);
                        FillRect(hdc, &rcBar, hBgBar);
                        DeleteObject(hBgBar);

                        if (ratio > 0) {
                            RECT rcProg = rcBar;
                            rcProg.right = rcBar.left + (int)((rcBar.right - rcBar.left) * ratio);
                            HBRUSH hProg = CreateSolidBrush(C7_GOLD);
                            FillRect(hdc, &rcProg, hProg);
                            DeleteObject(hProg);
                        }

                        // Progress Text
                        std::wstring progText = std::to_wstring((int)progress) + L"/" + std::to_wstring((int)threshold);
                        SetTextColor(hdc, C7_TEXT);
                        DrawTextW(hdc, progText.c_str(), -1, &rcBar, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                        SetBkMode(hdc, oldMode);
                        return TRUE;
                    }
                }

                // Standard Text
                wchar_t buf[256];
                SendMessageW(pDIS->hwndItem, LB_GETTEXT, pDIS->itemID, (LPARAM)buf);
                SetTextColor(hdc, (pDIS->itemState & ODS_SELECTED) ? C7_TEXT : C7_TEXT_DIM);
                int oldMode = SetBkMode(hdc, TRANSPARENT);
                DrawTextW(hdc, buf, -1, &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                SetBkMode(hdc, oldMode);
                return TRUE;
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, C7_TEXT);
            SetBkMode(hdc, TRANSPARENT);
            if ((HWND)lParam == m_eventLabel) {
                SetBkColor(hdc, C7_PANEL);
                return (LRESULT)m_panelBrush;
            }
            return (LRESULT)m_bgBrush;
        }
        case WM_CTLCOLORBTN: {
            return (LRESULT)m_bgBrush;
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
