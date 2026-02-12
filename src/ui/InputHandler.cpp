#include "ui/InputHandler.h"
#include "core/ColorOutput.h"
#include <iostream>
#include <limits>
#include <string>

namespace civ {

int InputHandler::getInt(const std::string& prompt, int min, int max) {
    int value;
    while (true) {
        std::cout << "  " << ColorOutput::cyan(prompt) << " [" << min << "-" << max << "]: ";
        if (std::cin >> value && value >= min && value <= max) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  " << ColorOutput::error(u8"Неверный ввод. Введите число от "
                  + std::to_string(min) + u8" до " + std::to_string(max) + ".") << "\n";
    }
}

double InputHandler::getDouble(const std::string& prompt, double min, double max) {
    double value;
    while (true) {
        std::cout << "  " << ColorOutput::cyan(prompt) << ": ";
        if (std::cin >> value && value >= min && value <= max) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  " << ColorOutput::error(u8"Неверный ввод.") << "\n";
    }
}

std::string InputHandler::getString(const std::string& prompt) {
    std::cout << "  " << ColorOutput::cyan(prompt) << ": ";
    std::string value;
    std::getline(std::cin, value);
    return value;
}

bool InputHandler::getYesNo(const std::string& prompt) {
    while (true) {
        std::cout << "  " << ColorOutput::cyan(prompt) << u8" (д/н): ";
        std::string input;
        std::getline(std::cin, input);
        if (!input.empty()) {
            char c = static_cast<char>(std::tolower(input[0]));
            if (c == 'y' || c == 'Y') return true;
            if (c == 'n' || c == 'N') return false;
            // Russian д/н
            // UTF-8: д = 0xD0 0xB4, н = 0xD0 0xBD
            if (input.size() >= 2) {
                unsigned char c0 = static_cast<unsigned char>(input[0]);
                unsigned char c1 = static_cast<unsigned char>(input[1]);
                if (c0 == 0xD0 && (c1 == 0xB4 || c1 == 0x94)) return true;  // д or Д
                if (c0 == 0xD0 && (c1 == 0xBD || c1 == 0x9D)) return false; // н or Н
            }
        }
        std::cout << "  " << ColorOutput::error(u8"Введите 'д' или 'н'.") << "\n";
    }
}

int InputHandler::getMenuChoice(const std::string& prompt, int numOptions) {
    return getInt(prompt, 1, numOptions);
}

void InputHandler::waitForKey() {
    std::cout << "\n  " << ColorOutput::dim(u8"Нажмите Enter для продолжения...") << std::flush;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

} // namespace civ
