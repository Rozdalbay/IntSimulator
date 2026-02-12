#include "core/Utils.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cmath>

namespace civ {

std::mt19937& Utils::getGenerator() {
    static std::mt19937 gen(
        static_cast<unsigned>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count()
        )
    );
    return gen;
}

int Utils::randomInt(int min, int max) {
    if (min > max) std::swap(min, max);
    std::uniform_int_distribution<int> dist(min, max);
    return dist(getGenerator());
}

double Utils::randomDouble(double min, double max) {
    if (min > max) std::swap(min, max);
    std::uniform_real_distribution<double> dist(min, max);
    return dist(getGenerator());
}

bool Utils::randomChance(double probability) {
    return randomDouble(0.0, 1.0) < probability;
}

std::string Utils::padRight(const std::string& str, size_t width, char fill) {
    if (str.size() >= width) return str;
    return str + std::string(width - str.size(), fill);
}

std::string Utils::padLeft(const std::string& str, size_t width, char fill) {
    if (str.size() >= width) return str;
    return std::string(width - str.size(), fill) + str;
}

std::string Utils::formatNumber(int64_t number) {
    std::string numStr = std::to_string(std::abs(number));
    std::string result;
    int count = 0;
    for (auto it = numStr.rbegin(); it != numStr.rend(); ++it) {
        if (count > 0 && count % 3 == 0) {
            result = ',' + result;
        }
        result = *it + result;
        ++count;
    }
    if (number < 0) {
        result = '-' + result;
    }
    return result;
}

std::string Utils::formatDouble(double value, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

std::string Utils::progressBar(double value, double maxValue, int width) {
    double ratio = (maxValue > 0.0) ? clamp(value / maxValue, 0.0, 1.0) : 0.0;
    int filled = static_cast<int>(std::round(ratio * width));
    int empty = width - filled;

    std::string bar = "[";
    for (int i = 0; i < filled; ++i) bar += '#';
    for (int i = 0; i < empty; ++i) bar += '-';
    bar += "]";

    return bar;
}

double Utils::clamp(double value, double min, double max) {
    return std::max(min, std::min(max, value));
}

std::string Utils::currentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
#ifdef _WIN32
    localtime_s(&tm_buf, &time);
#else
    localtime_r(&time, &tm_buf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

} // namespace civ
