#include "core/ColorOutput.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace civ {

bool ColorOutput::s_initialized = false;

void ColorOutput::init() {
    if (s_initialized) return;

#ifdef _WIN32
    // Enable ANSI escape codes on Windows 10+
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
        // Set console output to UTF-8
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    }
#endif

    s_initialized = true;
}

std::string ColorOutput::colorize(const std::string& code, const std::string& text) {
    return code + text + RESET;
}

std::string ColorOutput::red(const std::string& text) {
    return colorize("\033[31m", text);
}

std::string ColorOutput::green(const std::string& text) {
    return colorize("\033[32m", text);
}

std::string ColorOutput::yellow(const std::string& text) {
    return colorize("\033[33m", text);
}

std::string ColorOutput::blue(const std::string& text) {
    return colorize("\033[34m", text);
}

std::string ColorOutput::magenta(const std::string& text) {
    return colorize("\033[35m", text);
}

std::string ColorOutput::cyan(const std::string& text) {
    return colorize("\033[36m", text);
}

std::string ColorOutput::white(const std::string& text) {
    return colorize("\033[37m", text);
}

std::string ColorOutput::bold(const std::string& text) {
    return colorize("\033[1m", text);
}

std::string ColorOutput::dim(const std::string& text) {
    return colorize("\033[2m", text);
}

std::string ColorOutput::success(const std::string& text) {
    return green(text);
}

std::string ColorOutput::warning(const std::string& text) {
    return yellow(text);
}

std::string ColorOutput::error(const std::string& text) {
    return red(text);
}

std::string ColorOutput::info(const std::string& text) {
    return cyan(text);
}

std::string ColorOutput::highlight(const std::string& text) {
    return colorize("\033[1;33m", text);
}

} // namespace civ
