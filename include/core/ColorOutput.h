#pragma once

#include <string>

namespace civ {

/**
 * @brief Cross-platform colored console output using ANSI escape codes.
 *        On Windows, enables virtual terminal processing.
 */
class ColorOutput {
public:
    // Initialize console for color support (call once at startup)
    static void init();

    // Color codes
    static std::string red(const std::string& text);
    static std::string green(const std::string& text);
    static std::string yellow(const std::string& text);
    static std::string blue(const std::string& text);
    static std::string magenta(const std::string& text);
    static std::string cyan(const std::string& text);
    static std::string white(const std::string& text);
    static std::string bold(const std::string& text);
    static std::string dim(const std::string& text);

    // Semantic colors
    static std::string success(const std::string& text);
    static std::string warning(const std::string& text);
    static std::string error(const std::string& text);
    static std::string info(const std::string& text);
    static std::string highlight(const std::string& text);

    // Reset
    static constexpr const char* RESET = "\033[0m";

private:
    static bool s_initialized;
    static std::string colorize(const std::string& code, const std::string& text);
};

} // namespace civ
