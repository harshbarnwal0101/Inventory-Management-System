#pragma once
#include <string>

namespace IMS {

struct SaleRecord {
    int         saleId;
    int         productId;
    std::string productName;
    int         quantitySold;
    double      salePrice;      // Per unit at time of sale
    double      totalRevenue;
    std::string saleDate;
    std::string soldBy;         // username
};

} // namespace IMS
