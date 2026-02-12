#include "ui/Win32Gui.h"
#include <Windows.h>
#include <stdexcept>

/**
 * @brief WinMain entry point for Civilization Evolution Simulator (GUI version)
 */
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                   _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
    try {
        civ::Win32Gui gui;
        return gui.run(hInstance, nCmdShow);
    }
    catch (const std::exception& e) {
        MessageBoxA(nullptr, e.what(), "Fatal Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    catch (...) {
        MessageBoxA(nullptr, "Unknown fatal error occurred.", "Fatal Error", MB_OK | MB_ICONERROR);
        return 2;
    }
}
