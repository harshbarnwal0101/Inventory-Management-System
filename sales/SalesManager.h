#pragma once
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>
#include "../sales/SaleRecord.h"
#include "../utils/FileHandler.h"

namespace IMS {

class SalesManager {
public:
    explicit SalesManager(const std::string& file = "data/sales.csv")
        : file_(file), nextId_(1) {
        sales_ = FileHandler::loadSales(file_);
        for (const auto& s : sales_)
            if (s.saleId >= nextId_) nextId_ = s.saleId + 1;
    }

    SaleRecord recordSale(int productId, const std::string& productName,
                          int qty, double unitPrice,
                          const std::string& soldBy) {
        SaleRecord s;
        s.saleId       = nextId_++;
        s.productId    = productId;
        s.productName  = productName;
        s.quantitySold = qty;
        s.salePrice    = unitPrice;
        s.totalRevenue = qty * unitPrice;
        s.saleDate     = currentDateTime();
        s.soldBy       = soldBy;
        sales_.push_back(s);
        save();
        return s;
    }

    const std::vector<SaleRecord>& getAll() const { return sales_; }

    // ── Summaries ─────────────────────────────────────────────────────────
    double totalRevenue() const {
        double t = 0;
        for (const auto& s : sales_) t += s.totalRevenue;
        return t;
    }

    int totalUnitsSold() const {
        int t = 0;
        for (const auto& s : sales_) t += s.quantitySold;
        return t;
    }

    // Revenue grouped by date prefix (YYYY-MM-DD / YYYY-MM)
    std::map<std::string, double> revenueByDay() const {
        std::map<std::string, double> m;
        for (const auto& s : sales_) {
            std::string day = s.saleDate.substr(0, 10);
            m[day] += s.totalRevenue;
        }
        return m;
    }

    std::map<std::string, double> revenueByMonth() const {
        std::map<std::string, double> m;
        for (const auto& s : sales_) {
            std::string mon = s.saleDate.substr(0, 7);
            m[mon] += s.totalRevenue;
        }
        return m;
    }

    // Top N products by units sold
    struct ProductStat {
        std::string name;
        int         unitsSold;
        double      revenue;
    };

    std::vector<ProductStat> topProducts(int n = 5) const {
        std::map<int, ProductStat> agg;
        for (const auto& s : sales_) {
            agg[s.productId].name      = s.productName;
            agg[s.productId].unitsSold += s.quantitySold;
            agg[s.productId].revenue  += s.totalRevenue;
        }
        std::vector<ProductStat> v;
        for (auto& [id, ps] : agg) v.push_back(ps);
        std::sort(v.begin(), v.end(),
                  [](const ProductStat& a, const ProductStat& b) {
                      return a.unitsSold > b.unitsSold;
                  });
        if ((int)v.size() > n) v.resize(n);
        return v;
    }

    // Sales for a specific date prefix (e.g. "2025-07")
    std::vector<SaleRecord> filterByDate(const std::string& prefix) const {
        std::vector<SaleRecord> out;
        for (const auto& s : sales_)
            if (s.saleDate.substr(0, prefix.size()) == prefix)
                out.push_back(s);
        return out;
    }

private:
    std::string             file_;
    std::vector<SaleRecord> sales_;
    int                     nextId_;

    void save() { FileHandler::saveSales(sales_, file_); }

    static std::string currentDateTime() {
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
