#pragma once
#include <string>

namespace IMS {

struct Product {
    int         id;
    std::string name;
    std::string category;
    std::string supplier;
    double      price;          // Selling price
    double      costPrice;      // Purchase/cost price
    int         quantity;
    int         lowStockThreshold;
    std::string expiryDate;     // Optional, e.g. "2026-12-31" or "N/A"
    std::string addedDate;
    bool        isActive;       // Soft-delete flag

    bool isLowStock() const {
        return quantity <= lowStockThreshold;
    }

    double profitMargin() const {
        if (costPrice <= 0) return 0.0;
        return ((price - costPrice) / costPrice) * 100.0;
    }
};

} // namespace IMS
