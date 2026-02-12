#pragma once

#include <string>
#include <optional>

namespace civ {

/**
 * @brief Handles user input with validation.
 */
class InputHandler {
public:
    InputHandler() = default;

    // Get validated integer input
    [[nodiscard]] static int getInt(const std::string& prompt, int min, int max);

    // Get validated double input
    [[nodiscard]] static double getDouble(const std::string& prompt, double min, double max);

    // Get string input
    [[nodiscard]] static std::string getString(const std::string& prompt);

    // Get yes/no
    [[nodiscard]] static bool getYesNo(const std::string& prompt);

    // Get menu choice
    [[nodiscard]] static int getMenuChoice(const std::string& prompt, int numOptions);

    // Wait for any key
    static void waitForKey();
};

} // namespace civ
