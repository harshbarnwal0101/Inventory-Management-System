#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include "../auth/User.h"
#include "../inventory/Product.h"
#include "../sales/SaleRecord.h"

namespace IMS {
namespace FileHandler {

// ────────────────────────────────────────────────────────────────────────────
// CSV helpers
// ────────────────────────────────────────────────────────────────────────────
inline std::string escapeCSV(const std::string& s) {
    if (s.find(',') == std::string::npos &&
        s.find('"') == std::string::npos &&
        s.find('\n') == std::string::npos)
        return s;
    std::string out = "\"";
    for (char c : s) {
        if (c == '"') out += '"';
        out += c;
    }
    out += '"';
    return out;
}

inline std::vector<std::string> parseCSVLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string f;
    bool inQ = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (inQ) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i+1] == '"') { f += '"'; ++i; }
                else inQ = false;
            } else f += c;
        } else {
            if (c == '"') inQ = true;
            else if (c == ',') { fields.push_back(f); f.clear(); }
            else f += c;
        }
    }
    fields.push_back(f);
    return fields;
}

// ────────────────────────────────────────────────────────────────────────────
// Products
// ────────────────────────────────────────────────────────────────────────────
inline bool saveProducts(const std::vector<Product>& products, const std::string& path) {
    std::ofstream f(path);
    if (!f) return false;
    f << "id,name,category,supplier,price,costPrice,quantity,"
         "lowStockThreshold,expiryDate,addedDate,isActive\n";
    for (const auto& p : products) {
        f << p.id << ","
          << escapeCSV(p.name) << ","
          << escapeCSV(p.category) << ","
          << escapeCSV(p.supplier) << ","
          << std::fixed << std::setprecision(2) << p.price << ","
          << p.costPrice << ","
          << p.quantity << ","
          << p.lowStockThreshold << ","
          << escapeCSV(p.expiryDate) << ","
          << escapeCSV(p.addedDate) << ","
          << (p.isActive ? "1" : "0") << "\n";
    }
    return true;
}

inline std::vector<Product> loadProducts(const std::string& path) {
    std::vector<Product> products;
    std::ifstream f(path);
    if (!f) return products;
    std::string line;
    std::getline(f, line); // skip header
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        auto v = parseCSVLine(line);
        if (v.size() < 11) continue;
        Product p;
        try {
            p.id               = std::stoi(v[0]);
            p.name             = v[1];
            p.category         = v[2];
            p.supplier         = v[3];
            p.price            = std::stod(v[4]);
            p.costPrice        = std::stod(v[5]);
            p.quantity         = std::stoi(v[6]);
            p.lowStockThreshold= std::stoi(v[7]);
            p.expiryDate       = v[8];
            p.addedDate        = v[9];
            p.isActive         = (v[10] == "1");
        } catch (...) { continue; }
        products.push_back(p);
    }
    return products;
}

// ────────────────────────────────────────────────────────────────────────────
// Sales
// ────────────────────────────────────────────────────────────────────────────
inline bool saveSales(const std::vector<SaleRecord>& sales, const std::string& path) {
    std::ofstream f(path);
    if (!f) return false;
    f << "saleId,productId,productName,quantitySold,salePrice,totalRevenue,saleDate,soldBy\n";
    for (const auto& s : sales) {
        f << s.saleId << ","
          << s.productId << ","
          << escapeCSV(s.productName) << ","
          << s.quantitySold << ","
          << std::fixed << std::setprecision(2) << s.salePrice << ","
          << s.totalRevenue << ","
          << escapeCSV(s.saleDate) << ","
          << escapeCSV(s.soldBy) << "\n";
    }
    return true;
}

inline std::vector<SaleRecord> loadSales(const std::string& path) {
    std::vector<SaleRecord> sales;
    std::ifstream f(path);
    if (!f) return sales;
    std::string line;
    std::getline(f, line);
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        auto v = parseCSVLine(line);
        if (v.size() < 8) continue;
        SaleRecord s;
        try {
            s.saleId       = std::stoi(v[0]);
            s.productId    = std::stoi(v[1]);
            s.productName  = v[2];
            s.quantitySold = std::stoi(v[3]);
            s.salePrice    = std::stod(v[4]);
            s.totalRevenue = std::stod(v[5]);
            s.saleDate     = v[6];
            s.soldBy       = v[7];
        } catch (...) { continue; }
        sales.push_back(s);
    }
    return sales;
}

// ────────────────────────────────────────────────────────────────────────────
// Users (simple delimited binary-ish text, pipe-separated)
// ────────────────────────────────────────────────────────────────────────────
inline bool saveUsers(const std::vector<User>& users, const std::string& path) {
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;
    for (const auto& u : users) {
        f << u.username << "|"
          << u.passwordHash << "|"
          << u.roleStr() << "|"
          << u.lastLogin << "|"
          << (u.isActive ? "1" : "0") << "\n";
    }
    return true;
}

inline std::vector<User> loadUsers(const std::string& path) {
    std::vector<User> users;
    std::ifstream f(path, std::ios::binary);
    if (!f) return users;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::vector<std::string> v;
        std::string tok;
        while (std::getline(ss, tok, '|')) v.push_back(tok);
        if (v.size() < 5) continue;
        User u;
        u.username     = v[0];
        u.passwordHash = v[1];
        u.role         = (v[2] == "admin") ? Role::Admin : Role::Staff;
        u.lastLogin    = v[3];
        u.isActive     = (v[4] == "1" || v[4] == "1\r");
        users.push_back(u);
    }
    return users;
}

// ────────────────────────────────────────────────────────────────────────────
// Ensure directory exists (simple approach)
// ────────────────────────────────────────────────────────────────────────────
inline void ensureDataDir() {
#ifdef _WIN32
    system("if not exist data mkdir data");
#else
    system("mkdir -p data");
#endif
}

} // namespace FileHandler
} // namespace IMS
