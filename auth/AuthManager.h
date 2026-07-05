#pragma once
#include <string>
#include <vector>
#include <optional>
#include <functional>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <ctime>
#include "../auth/User.h"
#include "../utils/FileHandler.h"

namespace IMS {

class AuthManager {
public:
    explicit AuthManager(const std::string& usersFile = "data/users.dat")
        : usersFile_(usersFile) {
        users_ = FileHandler::loadUsers(usersFile_);
        seedDefaultAdmin();
    }

    // Returns the logged-in user on success, nullopt on failure
    std::optional<User> login(const std::string& username,
                              const std::string& password) {
        for (auto& u : users_) {
            if (u.username == username && u.isActive &&
                u.passwordHash == hashPassword(password)) {
                u.lastLogin = currentDateTime();
                save();
                return u;
            }
        }
        return std::nullopt;
    }

    bool addUser(const User& newUser) {
        for (const auto& u : users_)
            if (u.username == newUser.username) return false;
        users_.push_back(newUser);
        save();
        return true;
    }

    bool removeUser(const std::string& username) {
        for (auto& u : users_) {
            if (u.username == username) {
                u.isActive = false;
                save();
                return true;
            }
        }
        return false;
    }

    bool changePassword(const std::string& username,
                        const std::string& oldPwd,
                        const std::string& newPwd) {
        for (auto& u : users_) {
            if (u.username == username &&
                u.passwordHash == hashPassword(oldPwd)) {
                u.passwordHash = hashPassword(newPwd);
                save();
                return true;
            }
        }
        return false;
    }

    bool adminResetPassword(const std::string& username,
                            const std::string& newPwd) {
        for (auto& u : users_) {
            if (u.username == username) {
                u.passwordHash = hashPassword(newPwd);
                save();
                return true;
            }
        }
        return false;
    }

    std::vector<User> getUsers() const { return users_; }

    static std::string hashPassword(const std::string& pwd) {
        // Portable deterministic hash (not cryptographic, but sufficient for demos)
        uint64_t h = 0xcbf29ce484222325ULL;
        for (unsigned char c : pwd) {
            h ^= c;
            h *= 0x100000001b3ULL;
            h ^= (h >> 17);
            h *= 0xbf58476d1ce4e5b9ULL;
            h ^= (h >> 31);
        }
        // Convert to hex string
        std::ostringstream ss;
        ss << std::hex << std::setw(16) << std::setfill('0') << h;
        return ss.str();
    }

private:
    std::string       usersFile_;
    std::vector<User> users_;

    void save() { FileHandler::saveUsers(users_, usersFile_); }

    void seedDefaultAdmin() {
        if (users_.empty()) {
            User admin;
            admin.username     = "admin";
            admin.passwordHash = hashPassword("Admin@123");
            admin.role         = Role::Admin;
            admin.lastLogin    = "Never";
            admin.isActive     = true;
            users_.push_back(admin);
            save();
        }
    }

    std::string currentDateTime() {
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
