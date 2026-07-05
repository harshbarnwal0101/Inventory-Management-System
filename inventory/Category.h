#pragma once
#include <string>
#include <vector>

namespace IMS {

inline std::vector<std::string> getCategories() {
    return {
        "Electronics",
        "Clothing",
        "Food & Beverages",
        "Home & Garden",
        "Health & Beauty",
        "Sports & Outdoors",
        "Books & Stationery",
        "Toys & Games",
        "Automotive",
        "Other"
    };
}

inline bool isValidCategory(const std::string& cat) {
    for (const auto& c : getCategories())
        if (c == cat) return true;
    return false;
}

} // namespace IMS
