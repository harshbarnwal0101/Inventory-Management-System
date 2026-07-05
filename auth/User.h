#pragma once
#include <string>

namespace IMS {

enum class Role { Admin, Staff };

struct User {
    std::string username;
    std::string passwordHash;
    Role        role;
    std::string lastLogin;
    bool        isActive;

    std::string roleStr() const {
        return (role == Role::Admin) ? "admin" : "staff";
    }
};

} // namespace IMS
