#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace IMS {

class Logger {
public:
    static Logger& instance() {
        static Logger inst;
        return inst;
    }

    void setFile(const std::string& path) {
        logPath_ = path;
    }

    void log(const std::string& username, const std::string& action,
             const std::string& detail = "") {
        std::ofstream f(logPath_, std::ios::app);
        if (!f) return;
        f << "[" << now() << "] USER=" << username
          << " | ACTION=" << action;
        if (!detail.empty()) f << " | " << detail;
        f << "\n";
    }

private:
    Logger() = default;
    std::string logPath_ = "data/activity.log";

    std::string now() {
        auto t = std::time(nullptr);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        std::ostringstream ss;
        ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
};

} // namespace IMS
