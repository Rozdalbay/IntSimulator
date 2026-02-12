#pragma once

#include <random>
#include <string>

namespace civ {

/**
 * @brief Utility class providing random number generation and helper functions.
 */
class Utils {
public:
    // Random number generation
    static int randomInt(int min, int max);
    static double randomDouble(double min, double max);
    static bool randomChance(double probability); // probability in [0.0, 1.0]

    // String helpers
    static std::string padRight(const std::string& str, size_t width, char fill = ' ');
    static std::string padLeft(const std::string& str, size_t width, char fill = ' ');
    static std::string formatNumber(int64_t number);
    static std::string formatDouble(double value, int precision = 1);

    // Progress bar
    static std::string progressBar(double value, double maxValue, int width = 20);

    // Clamp
    static double clamp(double value, double min, double max);

    // Timestamp
    static std::string currentTimestamp();

private:
    static std::mt19937& getGenerator();
};

} // namespace civ
