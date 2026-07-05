#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <optional>
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include "Product.h"
#include "Category.h"
#include "../utils/FileHandler.h"

namespace IMS {

class Inventory {
public:
    explicit Inventory(const std::string& file = "data/inventory.csv")
        : file_(file), nextId_(1) {
        products_ = FileHandler::loadProducts(file_);
        for (const auto& p : products_)
            if (p.id >= nextId_) nextId_ = p.id + 1;
    }

    // ── CRUD ──────────────────────────────────────────────────────────────
    Product& addProduct(Product p) {
        p.id        = nextId_++;
        p.isActive  = true;
        p.addedDate = currentDate();
        products_.push_back(p);
        save();
        return products_.back();
    }

    bool updateProduct(int id, const Product& updated) {
        for (auto& p : products_) {
            if (p.id == id && p.isActive) {
                int keepId        = p.id;
                std::string keepDate = p.addedDate;
                p             = updated;
                p.id          = keepId;
                p.addedDate   = keepDate;
                save();
                return true;
            }
        }
        return false;
    }

    bool deleteProduct(int id) {
        for (auto& p : products_) {
            if (p.id == id && p.isActive) {
                p.isActive = false;
                save();
                return true;
            }
        }
        return false;
    }

    bool restockProduct(int id, int qty) {
        for (auto& p : products_) {
            if (p.id == id && p.isActive) {
                p.quantity += qty;
                save();
                return true;
            }
        }
        return false;
    }

    // Returns false if product not found, not active, or insufficient stock
    bool sellProduct(int id, int qty) {
        for (auto& p : products_) {
            if (p.id == id && p.isActive) {
                if (p.quantity < qty) return false;
                p.quantity -= qty;
                save();
                return true;
            }
        }
        return false;
    }

    // ── Queries ───────────────────────────────────────────────────────────
    std::optional<Product> findById(int id) const {
        for (const auto& p : products_)
            if (p.id == id && p.isActive) return p;
        return std::nullopt;
    }

    Product* findByIdMut(int id) {
        for (auto& p : products_)
            if (p.id == id && p.isActive) return &p;
        return nullptr;
    }

    std::vector<Product> getActive() const {
        std::vector<Product> out;
        for (const auto& p : products_)
            if (p.isActive) out.push_back(p);
        return out;
    }

    std::vector<Product> getLowStock() const {
        std::vector<Product> out;
        for (const auto& p : products_)
            if (p.isActive && p.isLowStock()) out.push_back(p);
        return out;
    }

    std::vector<Product> search(const std::string& query) const {
        std::string q = toLower(query);
        std::vector<Product> out;
        for (const auto& p : products_) {
            if (!p.isActive) continue;
            if (toLower(p.name).find(q) != std::string::npos ||
                toLower(p.category).find(q) != std::string::npos ||
                toLower(p.supplier).find(q) != std::string::npos ||
                std::to_string(p.id) == query)
                out.push_back(p);
        }
        return out;
    }

    std::vector<Product> getByCategory(const std::string& cat) const {
        std::vector<Product> out;
        for (const auto& p : products_)
            if (p.isActive && p.category == cat) out.push_back(p);
        return out;
    }

    // ── Sort ──────────────────────────────────────────────────────────────
    enum class SortBy { Name, Price, Quantity, Category };

    std::vector<Product> getSorted(SortBy by, bool ascending = true) const {
        auto items = getActive();
        auto cmp = [&](const Product& a, const Product& b) {
            bool res;
            switch (by) {
                case SortBy::Name:     res = a.name < b.name; break;
                case SortBy::Price:    res = a.price < b.price; break;
                case SortBy::Quantity: res = a.quantity < b.quantity; break;
                case SortBy::Category: res = a.category < b.category; break;
                default: res = a.id < b.id;
            }
            return ascending ? res : !res;
        };
        std::sort(items.begin(), items.end(), cmp);
        return items;
    }

    // ── Stats ─────────────────────────────────────────────────────────────
    int     totalProducts()    const { return (int)getActive().size(); }
    int     totalUnits()       const {
        int n = 0;
        for (const auto& p : products_) if (p.isActive) n += p.quantity;
        return n;
    }
    double  totalStockValue()  const {
        double v = 0;
        for (const auto& p : products_) if (p.isActive) v += p.costPrice * p.quantity;
        return v;
    }
    double  totalSaleValue()   const {
        double v = 0;
        for (const auto& p : products_) if (p.isActive) v += p.price * p.quantity;
        return v;
    }
    int     lowStockCount()    const { return (int)getLowStock().size(); }

private:
    std::string         file_;
    std::vector<Product> products_;
    int                  nextId_;

    void save() { FileHandler::saveProducts(products_, file_); }

    static std::string toLower(std::string s) {
        for (char& c : s) c = (char)std::tolower((unsigned char)c);
        return s;
    }

    static std::string currentDate() {
        auto t = std::time(nullptr);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        std::ostringstream ss;
        ss << std::put_time(&tm, "%Y-%m-%d");
        return ss.str();
    }
};

} // namespace IMS
