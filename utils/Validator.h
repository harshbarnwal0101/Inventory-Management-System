#pragma once
#include <string>
#include <sstream>
#include <regex>
#include <limits>
#include <iostream>
#ifdef _WIN32
#include <conio.h>
#endif

namespace IMS {
namespace Validator {

// Read a non-empty trimmed string from cin
inline std::string readString(const std::string& prompt) {
    std::string val;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, val);
        size_t s = val.find_first_not_of(" \t\r\n");
        size_t e = val.find_last_not_of(" \t\r\n");
        if (s != std::string::npos) {
            val = val.substr(s, e - s + 1);
            return val;
        }
        std::cout << "  [!] Input cannot be empty. Try again.\n";
    }
}

// Read an integer within [minVal, maxVal]
inline int readInt(const std::string& prompt, int minVal = 0, int maxVal = INT_MAX) {
    int val;
    while (true) {
        std::cout << prompt;
        std::string line;
        std::getline(std::cin, line);
        std::istringstream ss(line);
        if (ss >> val && val >= minVal && val <= maxVal) return val;
        std::cout << "  [!] Enter a number between " << minVal << " and " << maxVal << ".\n";
    }
}

// Read a non-negative double
inline double readDouble(const std::string& prompt, double minVal = 0.0) {
    double val;
    while (true) {
        std::cout << prompt;
        std::string line;
        std::getline(std::cin, line);
        std::istringstream ss(line);
        if (ss >> val && val >= minVal) return val;
        std::cout << "  [!] Enter a valid number >= " << minVal << ".\n";
    }
}

// Read a date string (YYYY-MM-DD) or "N/A"
inline std::string readDate(const std::string& prompt) {
    std::regex dateRe(R"(\d{4}-\d{2}-\d{2})");
    while (true) {
        std::cout << prompt;
        std::string val;
        std::getline(std::cin, val);
        if (val == "N/A" || std::regex_match(val, dateRe)) return val;
        std::cout << "  [!] Use YYYY-MM-DD format or 'N/A'.\n";
    }
}

// Read a password with asterisk masking on Windows, plain on others
inline std::string readPassword(const std::string& prompt) {
    std::cout << prompt;
    std::string pwd;
#ifdef _WIN32
    int ch;
    while ((ch = _getch()) != '\r' && ch != '\n') {
        if (ch == '\b' || ch == 127) {
            if (!pwd.empty()) { pwd.pop_back(); std::cout << "\b \b"; }
        } else if (ch >= 32) {
            pwd += static_cast<char>(ch);
            std::cout << '*';
        }
    }
    std::cout << '\n';
#else
    std::getline(std::cin, pwd);
#endif
    return pwd;
}

} // namespace Validator
} // namespace IMS
