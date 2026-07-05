#pragma once
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

namespace IMS {
namespace UI {

// ── ANSI Color Codes ────────────────────────────────────────────────────────
#ifdef _WIN32
#include <windows.h>
inline void enableColor() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hOut, &mode);
    SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}
#else
inline void enableColor() {}
#endif

const std::string RESET   = "\033[0m";
const std::string BOLD    = "\033[1m";
const std::string DIM     = "\033[2m";
const std::string RED     = "\033[31m";
const std::string GREEN   = "\033[32m";
const std::string YELLOW  = "\033[33m";
const std::string BLUE    = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN    = "\033[36m";
const std::string WHITE   = "\033[37m";
const std::string B_RED   = "\033[91m";
const std::string B_GREEN = "\033[92m";
const std::string B_YELLOW= "\033[93m";
const std::string B_CYAN  = "\033[96m";
const std::string BG_BLUE = "\033[44m";

// ── Clear Screen ────────────────────────────────────────────────────────────
inline void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// ── Banner ──────────────────────────────────────────────────────────────────
inline void printBanner() {
    std::cout << CYAN << BOLD;
    std::cout << R"(
  ╔══════════════════════════════════════════════════════════╗
  ║   ██╗███╗   ███╗███████╗     ██████╗ ██████╗  ██████╗   ║
  ║    ██║████╗ ████║██╔════╝    ██╔══██╗██╔══██╗██╔═══██╗  ║
  ║    ██║██╔████╔██║███████╗    ██████╔╝██████╔╝██║   ██║  ║
  ║    ██║██║╚██╔╝██║╚════██║    ██╔═══╝ ██╔══██╗██║   ██║  ║
  ║   ██╔╝██║ ╚═╝ ██║███████║    ██║     ██║  ██║╚██████╔╝  ║
  ║   ╚═╝ ╚═╝     ╚═╝╚══════╝    ╚═╝     ╚═╝  ╚═╝ ╚═════╝   ║
  ║          Inventory Management System  v1.0               ║
  ╚══════════════════════════════════════════════════════════╝)" << RESET << "\n\n";
}

// ── Section Headers ─────────────────────────────────────────────────────────
inline void printHeader(const std::string& title) {
    int width = 60;
    std::string bar(width, '=');
    std::cout << "\n" << CYAN << "  +" << bar << "+\n";
    int pad = (width - (int)title.size()) / 2;
    std::cout << "  |" << std::string(pad, ' ') << BOLD << title
              << RESET << CYAN << std::string(width - pad - (int)title.size(), ' ') << "|\n";
    std::cout << "  +" << bar << "+" << RESET << "\n\n";
}

// ── Success / Error / Warning messages ──────────────────────────────────────
inline void success(const std::string& msg) {
    std::cout << B_GREEN << "  ✔  " << msg << RESET << "\n";
}
inline void error(const std::string& msg) {
    std::cout << B_RED << "  ✘  " << msg << RESET << "\n";
}
inline void warn(const std::string& msg) {
    std::cout << B_YELLOW << "  ⚠  " << msg << RESET << "\n";
}
inline void info(const std::string& msg) {
    std::cout << B_CYAN << "  ℹ  " << msg << RESET << "\n";
}

// ── Horizontal rule ──────────────────────────────────────────────────────────
inline void hr(char c = '-', int width = 64) {
    std::cout << DIM << "  " << std::string(width, c) << RESET << "\n";
}

// ── Pause / Press Enter ──────────────────────────────────────────────────────
inline void pause() {
    std::cout << DIM << "\n  Press [Enter] to continue..." << RESET;
    std::cin.ignore(1000, '\n');
    // If cin was just used with >>, the ignore above consumes leftover newline.
    // Call a second time only if the stream is not at newline already.
}

// ── Table helpers ─────────────────────────────────────────────────────────
struct ColDef {
    std::string header;
    int         width;
    bool        rightAlign = false;
};

inline void printTableHeader(const std::vector<ColDef>& cols) {
    std::cout << BOLD << CYAN << "  ";
    for (const auto& c : cols) {
        if (c.rightAlign)
            std::cout << std::right << std::setw(c.width) << c.header << "  ";
        else
            std::cout << std::left  << std::setw(c.width) << c.header << "  ";
    }
    std::cout << RESET << "\n";
    // underline
    std::cout << "  ";
    for (const auto& c : cols)
        std::cout << std::string(c.width, '-') << "  ";
    std::cout << "\n";
}

inline void printTableRow(const std::vector<ColDef>& cols,
                          const std::vector<std::string>& vals,
                          const std::string& rowColor = "") {
    std::cout << (rowColor.empty() ? WHITE : rowColor) << "  ";
    for (size_t i = 0; i < cols.size(); ++i) {
        std::string v = (i < vals.size()) ? vals[i] : "";
        if ((int)v.size() > cols[i].width - 1)
            v = v.substr(0, cols[i].width - 4) + "...";
        if (cols[i].rightAlign)
            std::cout << std::right << std::setw(cols[i].width) << v << "  ";
        else
            std::cout << std::left  << std::setw(cols[i].width) << v << "  ";
    }
    std::cout << RESET << "\n";
}

// ── Stat box (dashboard) ────────────────────────────────────────────────────
inline void printStatBox(const std::string& label, const std::string& value,
                         const std::string& color = "") {
    std::string col = color.empty() ? CYAN : color;
    std::cout << col << "  ┌──────────────────────────┐\n";
    std::cout << "  │  " << BOLD << std::left << std::setw(24) << label << RESET << col << "│\n";
    std::cout << "  │  " << BOLD << WHITE << std::setw(24) << value << RESET << col << "│\n";
    std::cout << "  └──────────────────────────┘" << RESET << "\n";
}

// ── Menu option printer ───────────────────────────────────────────────────
inline void printMenuOpt(const std::string& key, const std::string& label) {
    std::cout << "    " << BOLD << CYAN << "[" << key << "]" << RESET
              << "  " << label << "\n";
}

} // namespace UI
} // namespace IMS
