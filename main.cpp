/*
 * Inventory Management System — main.cpp
 * Entry point, menu routing, and UI interactions
 */

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <optional>
#include <limits>

// Project headers
#include "auth/User.h"
#include "auth/AuthManager.h"
#include "inventory/Product.h"
#include "inventory/Category.h"
#include "inventory/Inventory.h"
#include "sales/SaleRecord.h"
#include "sales/SalesManager.h"
#include "utils/FileHandler.h"
#include "utils/Logger.h"
#include "utils/UIHelper.h"
#include "utils/Validator.h"

using namespace IMS;
using namespace IMS::UI;
using namespace IMS::Validator;

// ── Globals ──────────────────────────────────────────────────────────────────
AuthManager  authMgr;
Inventory    inventory;
SalesManager salesMgr;
User         currentUser;

// ── Helpers ───────────────────────────────────────────────────────────────────
static std::string fmtMoney(double v) {
    std::ostringstream ss;
    ss << "Rs." << std::fixed << std::setprecision(2) << v;
    return ss.str();
}

static void printProductTable(const std::vector<Product>& prods) {
    if (prods.empty()) { warn("No products to display."); return; }
    std::vector<ColDef> cols = {
        {"ID",         5,  true},
        {"Name",       22, false},
        {"Category",   18, false},
        {"Supplier",   14, false},
        {"Price",       9, true},
        {"Cost",        9, true},
        {"Qty",         5, true},
        {"Low?",        5, false},
    };
    printTableHeader(cols);
    for (const auto& p : prods) {
        std::string lowFlag = p.isLowStock() ? (B_RED + "YES" + RESET) : "no";
        std::string rowCol  = p.isLowStock() ? B_YELLOW : WHITE;
        printTableRow(cols, {
            std::to_string(p.id),
            p.name,
            p.category,
            p.supplier,
            fmtMoney(p.price),
            fmtMoney(p.costPrice),
            std::to_string(p.quantity),
            p.isLowStock() ? "YES" : "no"
        }, rowCol);
    }
    std::cout << "\n";
}

// ── Dashboard ─────────────────────────────────────────────────────────────────
static void showDashboard() {
    clearScreen();
    printBanner();
    printHeader("DASHBOARD");

    std::cout << "  " << DIM << "Logged in as: " << RESET
              << BOLD << currentUser.username << RESET
              << "  (" << (currentUser.role == Role::Admin ? "Admin" : "Staff") << ")\n\n";

    // Stat boxes in a row
    printStatBox("Total Products",  std::to_string(inventory.totalProducts()), CYAN);
    printStatBox("Total Units",     std::to_string(inventory.totalUnits()),    BLUE);
    printStatBox("Stock Value",     fmtMoney(inventory.totalStockValue()),     GREEN);
    printStatBox("Sale Value",      fmtMoney(inventory.totalSaleValue()),      MAGENTA);
    printStatBox("Total Revenue",   fmtMoney(salesMgr.totalRevenue()),         B_GREEN);
    printStatBox("Units Sold",      std::to_string(salesMgr.totalUnitsSold()), B_CYAN);

    // Low-stock alerts
    auto ls = inventory.getLowStock();
    if (!ls.empty()) {
        std::cout << "\n";
        warn("LOW STOCK ALERTS (" + std::to_string(ls.size()) + " items):");
        printProductTable(ls);
    } else {
        std::cout << "\n";
        success("All products are adequately stocked.");
    }
    pause();
}

// ═══════════════════════════════════════════════════════════════════
//  PRODUCT MANAGEMENT
// ═══════════════════════════════════════════════════════════════════

static void addProduct() {
    printHeader("ADD PRODUCT");
    Product p;
    p.name     = readString("  Product Name      : ");
    // Show categories
    auto cats = getCategories();
    std::cout << "\n  Categories:\n";
    for (int i = 0; i < (int)cats.size(); ++i)
        std::cout << "    [" << i+1 << "] " << cats[i] << "\n";
    int ci = readInt("  Choose category # : ", 1, (int)cats.size());
    p.category = cats[ci - 1];

    p.supplier          = readString("  Supplier           : ");
    p.price             = readDouble("  Selling Price (Rs.): ", 0.01);
    p.costPrice         = readDouble("  Cost Price   (Rs.) : ", 0.01);
    p.quantity          = readInt   ("  Initial Qty        : ", 0);
    p.lowStockThreshold = readInt   ("  Low Stock Threshold: ", 0);
    p.expiryDate        = readDate  ("  Expiry (YYYY-MM-DD/N/A): ");

    auto& added = inventory.addProduct(p);
    Logger::instance().log(currentUser.username, "ADD_PRODUCT",
                           "ID=" + std::to_string(added.id) + " Name=" + added.name);
    success("Product '" + added.name + "' added with ID " + std::to_string(added.id));
    pause();
}

static void viewAllProducts() {
    printHeader("ALL PRODUCTS");
    auto sort_choice = readInt(
        "  Sort by: [1]Name [2]Price [3]Qty [4]Category  : ", 1, 4);
    bool asc = (readInt("  Order:  [1]Ascending [2]Descending          : ", 1, 2) == 1);
    Inventory::SortBy sb;
    switch (sort_choice) {
        case 2: sb = Inventory::SortBy::Price;    break;
        case 3: sb = Inventory::SortBy::Quantity; break;
        case 4: sb = Inventory::SortBy::Category; break;
        default:sb = Inventory::SortBy::Name;
    }
    auto prods = inventory.getSorted(sb, asc);
    std::cout << "\n  " << BOLD << prods.size() << " product(s) found.\n\n" << RESET;
    printProductTable(prods);
    pause();
}

static void searchProducts() {
    printHeader("SEARCH PRODUCTS");
    std::string q = readString("  Enter search term (name/category/supplier/ID): ");
    auto results = inventory.search(q);
    std::cout << "\n  " << BOLD << results.size() << " result(s) for \"" << q << "\"\n\n" << RESET;
    printProductTable(results);
    pause();
}

static void updateProduct() {
    printHeader("UPDATE PRODUCT");
    int id = readInt("  Enter Product ID to update: ", 1);
    auto opt = inventory.findById(id);
    if (!opt) { error("Product ID " + std::to_string(id) + " not found."); pause(); return; }

    Product p = *opt;
    std::cout << "\n  Current: " << BOLD << p.name << RESET << " | "
              << p.category << " | Qty:" << p.quantity << " | Price:" << fmtMoney(p.price) << "\n\n";

    info("Press Enter to keep current value.");

    auto readOpt = [](const std::string& prompt, const std::string& cur) -> std::string {
        std::cout << prompt << " [" << cur << "]: ";
        std::string v;
        std::getline(std::cin, v);
        if (v.empty()) return cur;
        size_t s = v.find_first_not_of(" \t");
        return (s != std::string::npos) ? v.substr(s) : cur;
    };

    p.name     = readOpt("  Name    ", p.name);
    p.supplier = readOpt("  Supplier", p.supplier);

    std::string priceS = readOpt("  Price  ", std::to_string(p.price));
    try { p.price = std::stod(priceS); } catch (...) {}
    std::string costS = readOpt("  Cost   ", std::to_string(p.costPrice));
    try { p.costPrice = std::stod(costS); } catch (...) {}

    std::string qtyS = readOpt("  Qty    ", std::to_string(p.quantity));
    try { p.quantity = std::stoi(qtyS); } catch (...) {}
    std::string thrS = readOpt("  LowThr ", std::to_string(p.lowStockThreshold));
    try { p.lowStockThreshold = std::stoi(thrS); } catch (...) {}

    p.expiryDate = readOpt("  Expiry ", p.expiryDate);

    inventory.updateProduct(id, p);
    Logger::instance().log(currentUser.username, "UPDATE_PRODUCT", "ID=" + std::to_string(id));
    success("Product updated.");
    pause();
}

static void deleteProduct() {
    printHeader("DELETE PRODUCT");
    int id = readInt("  Enter Product ID to delete: ", 1);
    auto opt = inventory.findById(id);
    if (!opt) { error("Product not found."); pause(); return; }

    std::cout << "  " << BOLD << "About to soft-delete: " << opt->name << RESET << "\n";
    std::string conf = readString("  Type 'yes' to confirm: ");
    if (conf != "yes") { warn("Cancelled."); pause(); return; }

    inventory.deleteProduct(id);
    Logger::instance().log(currentUser.username, "DELETE_PRODUCT",
                           "ID=" + std::to_string(id) + " Name=" + opt->name);
    success("Product '" + opt->name + "' deleted (soft).");
    pause();
}

static void productMenu() {
    while (true) {
        clearScreen();
        printHeader("PRODUCT MANAGEMENT");
        printMenuOpt("1", "Add Product");
        printMenuOpt("2", "View All Products");
        printMenuOpt("3", "Search / Filter");
        printMenuOpt("4", "Update Product");
        printMenuOpt("5", "Delete Product");
        printMenuOpt("0", "Back");
        int ch = readInt("\n  Choice: ", 0, 5);
        switch (ch) {
            case 1: clearScreen(); addProduct();    break;
            case 2: clearScreen(); viewAllProducts(); break;
            case 3: clearScreen(); searchProducts(); break;
            case 4: clearScreen(); updateProduct(); break;
            case 5: clearScreen(); deleteProduct(); break;
            case 0: return;
        }
    }
}

// ═══════════════════════════════════════════════════════════════════
//  STOCK MANAGEMENT
// ═══════════════════════════════════════════════════════════════════

static void restockProduct() {
    printHeader("RESTOCK PRODUCT");
    int id  = readInt("  Product ID : ", 1);
    auto opt = inventory.findById(id);
    if (!opt) { error("Product not found."); pause(); return; }
    std::cout << "  " << BOLD << opt->name << RESET << " — Current Qty: " << opt->quantity << "\n";
    int qty = readInt("  Add Qty    : ", 1);
    inventory.restockProduct(id, qty);
    Logger::instance().log(currentUser.username, "RESTOCK",
                           "ID=" + std::to_string(id) + " +Qty=" + std::to_string(qty));
    success("Restocked. New quantity: " + std::to_string(opt->quantity + qty));
    pause();
}

static void recordSale() {
    printHeader("RECORD SALE");
    int id  = readInt("  Product ID : ", 1);
    auto opt = inventory.findById(id);
    if (!opt) { error("Product not found."); pause(); return; }
    std::cout << "  " << BOLD << opt->name << RESET
              << " — Price: " << fmtMoney(opt->price)
              << "  Stock: " << opt->quantity << "\n";
    int qty = readInt("  Qty to sell: ", 1);
    if (opt->quantity < qty) { error("Insufficient stock."); pause(); return; }

    inventory.sellProduct(id, qty);
    auto sale = salesMgr.recordSale(id, opt->name, qty, opt->price, currentUser.username);
    Logger::instance().log(currentUser.username, "SALE",
                           "SaleID=" + std::to_string(sale.saleId) +
                           " ProductID=" + std::to_string(id) +
                           " Qty=" + std::to_string(qty));
    success("Sale recorded! Revenue: " + fmtMoney(sale.totalRevenue));
    pause();
}

static void lowStockReport() {
    printHeader("LOW STOCK REPORT");
    auto ls = inventory.getLowStock();
    if (ls.empty()) { success("No low-stock products."); pause(); return; }
    warn(std::to_string(ls.size()) + " product(s) need restocking:");
    printProductTable(ls);
    pause();
}

static void stockMenu() {
    while (true) {
        clearScreen();
        printHeader("STOCK MANAGEMENT");
        printMenuOpt("1", "Restock Product");
        printMenuOpt("2", "Record Sale");
        printMenuOpt("3", "Low Stock Report");
        printMenuOpt("0", "Back");
        int ch = readInt("\n  Choice: ", 0, 3);
        switch (ch) {
            case 1: clearScreen(); restockProduct(); break;
            case 2: clearScreen(); recordSale();     break;
            case 3: clearScreen(); lowStockReport(); break;
            case 0: return;
        }
    }
}

// ═══════════════════════════════════════════════════════════════════
//  SALES & REPORTS
// ═══════════════════════════════════════════════════════════════════

static void viewSalesHistory() {
    printHeader("SALES HISTORY");
    std::cout << "  Filter by date prefix (YYYY-MM-DD or YYYY-MM) or press Enter for all: ";
    std::string pref;
    std::getline(std::cin, pref);

    std::vector<SaleRecord> records;
    if (pref.empty()) records = salesMgr.getAll();
    else              records = salesMgr.filterByDate(pref);

    if (records.empty()) { warn("No sales found."); pause(); return; }

    std::vector<ColDef> cols = {
        {"SaleID", 7, true}, {"ProductID", 10, true}, {"Product", 20, false},
        {"Qty", 5, true}, {"Unit Price", 11, true}, {"Revenue", 12, true},
        {"Date", 19, false}, {"By", 10, false}
    };
    printTableHeader(cols);
    double total = 0;
    for (const auto& s : records) {
        total += s.totalRevenue;
        printTableRow(cols, {
            std::to_string(s.saleId),
            std::to_string(s.productId),
            s.productName,
            std::to_string(s.quantitySold),
            fmtMoney(s.salePrice),
            fmtMoney(s.totalRevenue),
            s.saleDate,
            s.soldBy
        });
    }
    hr();
    std::cout << "  " << BOLD << "Total: " << fmtMoney(total) << RESET << "\n\n";
    pause();
}

static void revenueReport() {
    printHeader("REVENUE REPORT (BY MONTH)");
    auto byMonth = salesMgr.revenueByMonth();
    if (byMonth.empty()) { warn("No sales data."); pause(); return; }
    std::vector<ColDef> cols = {{"Month", 10, false}, {"Revenue", 15, true}};
    printTableHeader(cols);
    for (auto& [mon, rev] : byMonth)
        printTableRow(cols, {mon, fmtMoney(rev)});
    std::cout << "\n";
    pause();
}

static void topProductsReport() {
    printHeader("TOP 5 SELLING PRODUCTS");
    auto top = salesMgr.topProducts(5);
    if (top.empty()) { warn("No sales data."); pause(); return; }
    std::vector<ColDef> cols = {
        {"Product", 25, false}, {"Units Sold", 12, true}, {"Revenue", 14, true}
    };
    printTableHeader(cols);
    for (const auto& ps : top)
        printTableRow(cols, {ps.name, std::to_string(ps.unitsSold), fmtMoney(ps.revenue)});
    std::cout << "\n";
    pause();
}

static void profitAnalysis() {
    printHeader("PROFIT ANALYSIS");
    auto prods = inventory.getActive();
    if (prods.empty()) { warn("No products."); pause(); return; }
    std::vector<ColDef> cols = {
        {"Product", 22, false}, {"Cost", 10, true}, {"Price", 10, true},
        {"Margin%", 9, true}, {"Qty", 5, true}, {"Est. Profit", 13, true}
    };
    printTableHeader(cols);
    for (const auto& p : prods) {
        double estProfit = (p.price - p.costPrice) * p.quantity;
        std::ostringstream mar;
        mar << std::fixed << std::setprecision(1) << p.profitMargin() << "%";
        std::string rowCol = p.profitMargin() < 0 ? B_RED : WHITE;
        printTableRow(cols, {
            p.name,
            fmtMoney(p.costPrice),
            fmtMoney(p.price),
            mar.str(),
            std::to_string(p.quantity),
            fmtMoney(estProfit)
        }, rowCol);
    }
    std::cout << "\n";
    pause();
}

static void reportsMenu() {
    while (true) {
        clearScreen();
        printHeader("SALES & REPORTS");
        printMenuOpt("1", "Sales History");
        printMenuOpt("2", "Revenue by Month");
        printMenuOpt("3", "Top 5 Products");
        printMenuOpt("4", "Profit Analysis");
        printMenuOpt("0", "Back");
        int ch = readInt("\n  Choice: ", 0, 4);
        switch (ch) {
            case 1: clearScreen(); viewSalesHistory();  break;
            case 2: clearScreen(); revenueReport();     break;
            case 3: clearScreen(); topProductsReport(); break;
            case 4: clearScreen(); profitAnalysis();    break;
            case 0: return;
        }
    }
}

// ═══════════════════════════════════════════════════════════════════
//  USER MANAGEMENT  (Admin only)
// ═══════════════════════════════════════════════════════════════════

static void viewUsers() {
    printHeader("USER LIST");
    auto users = authMgr.getUsers();
    std::vector<ColDef> cols = {
        {"Username", 16, false}, {"Role", 8, false},
        {"Active", 7, false}, {"Last Login", 20, false}
    };
    printTableHeader(cols);
    for (const auto& u : users)
        printTableRow(cols, {
            u.username, u.roleStr(),
            u.isActive ? "Yes" : "No", u.lastLogin
        });
    std::cout << "\n";
    pause();
}

static void addUser() {
    printHeader("ADD USER");
    std::string uname = readString("  Username : ");
    std::string pwd   = readPassword("  Password : ");
    int rc = readInt("  Role [1]Admin [2]Staff: ", 1, 2);
    User u;
    u.username     = uname;
    u.passwordHash = AuthManager::hashPassword(pwd);
    u.role         = (rc == 1) ? Role::Admin : Role::Staff;
    u.lastLogin    = "Never";
    u.isActive     = true;
    if (authMgr.addUser(u)) {
        Logger::instance().log(currentUser.username, "ADD_USER", "new=" + uname);
        success("User '" + uname + "' created.");
    } else {
        error("Username already exists.");
    }
    pause();
}

static void removeUser() {
    printHeader("REMOVE USER");
    std::string uname = readString("  Username to remove: ");
    if (uname == currentUser.username) { error("Cannot remove yourself."); pause(); return; }
    std::string conf = readString("  Confirm 'yes': ");
    if (conf != "yes") { warn("Cancelled."); pause(); return; }
    if (authMgr.removeUser(uname)) {
        Logger::instance().log(currentUser.username, "REMOVE_USER", "target=" + uname);
        success("User removed.");
    } else {
        error("User not found.");
    }
    pause();
}

static void resetPassword() {
    printHeader("RESET USER PASSWORD");
    std::string uname = readString("  Username       : ");
    std::string newPwd = readPassword("  New Password   : ");
    if (authMgr.adminResetPassword(uname, newPwd)) {
        Logger::instance().log(currentUser.username, "RESET_PASSWORD", "target=" + uname);
        success("Password reset for " + uname);
    } else {
        error("User not found.");
    }
    pause();
}

static void userMgmtMenu() {
    while (true) {
        clearScreen();
        printHeader("USER MANAGEMENT");
        printMenuOpt("1", "View Users");
        printMenuOpt("2", "Add User");
        printMenuOpt("3", "Remove User");
        printMenuOpt("4", "Reset Password");
        printMenuOpt("0", "Back");
        int ch = readInt("\n  Choice: ", 0, 4);
        switch (ch) {
            case 1: clearScreen(); viewUsers();     break;
            case 2: clearScreen(); addUser();       break;
            case 3: clearScreen(); removeUser();    break;
            case 4: clearScreen(); resetPassword(); break;
            case 0: return;
        }
    }
}

// ═══════════════════════════════════════════════════════════════════
//  ACTIVITY LOG
// ═══════════════════════════════════════════════════════════════════

static void viewActivityLog() {
    printHeader("ACTIVITY LOG");
    std::ifstream f("data/activity.log");
    if (!f) { warn("No activity log found."); pause(); return; }
    std::string line;
    int count = 0;
    std::vector<std::string> lines;
    while (std::getline(f, line)) lines.push_back(line);
    // Show last 30 entries
    int start = (int)lines.size() > 30 ? (int)lines.size() - 30 : 0;
    for (int i = start; i < (int)lines.size(); ++i) {
        std::cout << "  " << DIM << lines[i] << RESET << "\n";
        ++count;
    }
    if (count == 0) warn("Log is empty.");
    std::cout << "\n";
    pause();
}

// ═══════════════════════════════════════════════════════════════════
//  CHANGE MY PASSWORD
// ═══════════════════════════════════════════════════════════════════

static void changeMyPassword() {
    printHeader("CHANGE PASSWORD");
    std::string old = readPassword("  Current Password: ");
    std::string n1  = readPassword("  New Password    : ");
    std::string n2  = readPassword("  Confirm New     : ");
    if (n1 != n2) { error("Passwords don't match."); pause(); return; }
    if (authMgr.changePassword(currentUser.username, old, n1)) {
        Logger::instance().log(currentUser.username, "CHANGE_PASSWORD", "");
        success("Password changed.");
    } else {
        error("Current password incorrect.");
    }
    pause();
}

// ═══════════════════════════════════════════════════════════════════
//  MAIN MENUS
// ═══════════════════════════════════════════════════════════════════

static void adminMenu() {
    while (true) {
        clearScreen();
        printBanner();
        std::cout << "  " << BOLD << CYAN << currentUser.username
                  << RESET << "  (Admin)\n\n";
        printMenuOpt("1", "Dashboard");
        printMenuOpt("2", "Product Management");
        printMenuOpt("3", "Stock Management");
        printMenuOpt("4", "Sales & Reports");
        printMenuOpt("5", "User Management");
        printMenuOpt("6", "Activity Log");
        printMenuOpt("7", "Change My Password");
        printMenuOpt("0", "Logout");
        int ch = readInt("\n  Choice: ", 0, 7);
        switch (ch) {
            case 1: showDashboard();    break;
            case 2: productMenu();      break;
            case 3: stockMenu();        break;
            case 4: reportsMenu();      break;
            case 5: userMgmtMenu();     break;
            case 6: clearScreen(); viewActivityLog(); break;
            case 7: clearScreen(); changeMyPassword(); break;
            case 0:
                Logger::instance().log(currentUser.username, "LOGOUT", "");
                return;
        }
    }
}

static void staffMenu() {
    while (true) {
        clearScreen();
        printBanner();
        std::cout << "  " << BOLD << CYAN << currentUser.username
                  << RESET << "  (Staff)\n\n";
        printMenuOpt("1", "Dashboard");
        printMenuOpt("2", "View Products");
        printMenuOpt("3", "Search Products");
        printMenuOpt("4", "Record Sale");
        printMenuOpt("5", "Restock Product");
        printMenuOpt("6", "Change My Password");
        printMenuOpt("0", "Logout");
        int ch = readInt("\n  Choice: ", 0, 6);
        switch (ch) {
            case 1: showDashboard();               break;
            case 2: clearScreen(); viewAllProducts(); break;
            case 3: clearScreen(); searchProducts(); break;
            case 4: clearScreen(); recordSale();    break;
            case 5: clearScreen(); restockProduct(); break;
            case 6: clearScreen(); changeMyPassword(); break;
            case 0:
                Logger::instance().log(currentUser.username, "LOGOUT", "");
                return;
        }
    }
}

// ═══════════════════════════════════════════════════════════════════
//  LOGIN SCREEN
// ═══════════════════════════════════════════════════════════════════

static bool loginScreen() {
    int attempts = 0;
    while (attempts < 3) {
        clearScreen();
        printBanner();
        printHeader("LOGIN");
        std::string uname = readString("  Username : ");
        std::string pwd   = readPassword("  Password : ");

        auto opt = authMgr.login(uname, pwd);
        if (opt) {
            currentUser = *opt;
            Logger::instance().log(currentUser.username, "LOGIN", "SUCCESS");
            success("Welcome, " + currentUser.username + "!");
            std::cout << "\n";
            return true;
        }
        ++attempts;
        error("Invalid credentials. Attempt " + std::to_string(attempts) + "/3");
        Logger::instance().log(uname, "LOGIN_FAIL", "Attempt " + std::to_string(attempts));
        if (attempts < 3) pause();
    }
    error("Too many failed attempts. System locked.");
    return false;
}

// ═══════════════════════════════════════════════════════════════════
//  ENTRY POINT
// ═══════════════════════════════════════════════════════════════════

int main() {
    enableColor();
    FileHandler::ensureDataDir();
    Logger::instance().setFile("data/activity.log");

    clearScreen();
    printBanner();
    info("Welcome to IMS — Retail Inventory Management System");
    info("Default admin login: admin / Admin@123");
    std::cout << "\n";
    pause();

    while (true) {
        if (!loginScreen()) break;

        if (currentUser.role == Role::Admin) adminMenu();
        else                                 staffMenu();

        clearScreen();
        printBanner();
        std::cout << "\n  " << BOLD << "Logged out.\n" << RESET;
        printMenuOpt("1", "Login again");
        printMenuOpt("0", "Exit");
        int ch = readInt("\n  Choice: ", 0, 1);
        if (ch == 0) break;
    }

    clearScreen();
    printBanner();
    success("Thank you for using IMS. Goodbye!");
    std::cout << "\n";
    return 0;
}
