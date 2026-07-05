# Inventory Management System (IMS)

A full-featured, console-based **Inventory Management System** built in **C++17** for retail stores.

## Features

| Module | Capabilities |
|--------|-------------|
| **Authentication** | Multi-role login (Admin / Staff), password hashing, 3-attempt lockout, default admin seeding |
| **Product Management** | Add, view, update, soft-delete, search (name/category/supplier/ID), sort (name/price/qty/category) |
| **Stock Management** | Restock products, record sales, low-stock alerts with configurable threshold |
| **Sales & Reports** | Sales history with date filter, revenue by month, top-5 products, profit margin analysis |
| **User Management** | Add/remove users, reset passwords, role assignment (Admin only) |
| **Audit Logging** | Every action timestamped and written to `data/activity.log` |
| **Persistence** | Products → `data/inventory.csv`, Sales → `data/sales.csv`, Users → `data/users.dat` |

## Project Structure

```
InventoryMS/
├── main.cpp                  # Entry point, all menus and UI logic
├── auth/
│   ├── User.h                # User struct + Role enum
│   └── AuthManager.h         # Login, hashing, user CRUD
├── inventory/
│   ├── Product.h             # Product struct
│   ├── Category.h            # Category list helpers
│   └── Inventory.h           # Full CRUD + search + sort + stats
├── sales/
│   ├── SaleRecord.h          # Sale transaction struct
│   └── SalesManager.h        # Record sales, reports, revenue stats
├── utils/
│   ├── FileHandler.h         # CSV + binary file I/O
│   ├── Logger.h              # Singleton audit logger
│   ├── UIHelper.h            # ANSI colors, banner, tables, menus
│   └── Validator.h           # Type-safe console input helpers
├── data/                     # Auto-created at runtime
│   ├── inventory.csv
│   ├── sales.csv
│   ├── users.dat
│   └── activity.log
├── Makefile
└── README.md
```

## Build & Run

### Requirements
- **g++ with C++17 support** (MinGW-w64 on Windows, or GCC/Clang on Linux/macOS)

### Windows (PowerShell / CMD)
```powershell
g++ -std=c++17 -Wall -O2 -o ims main.cpp
.\ims.exe
```

### Linux / macOS
```bash
make
./ims
```

## Default Credentials

| Username | Password  | Role  |
|----------|-----------|-------|
| `admin`  | `Admin@123` | Admin |

> Change the password after first login via **Change My Password** in the menu.

## Menu Structure

```
Admin Menu
├── 1. Dashboard         — Stats overview + low-stock alerts
├── 2. Product Management
│   ├── Add / View / Search / Update / Delete
├── 3. Stock Management
│   ├── Restock / Record Sale / Low Stock Report
├── 4. Sales & Reports
│   ├── History / Revenue by Month / Top Products / Profit Analysis
├── 5. User Management   — Add/Remove users, reset passwords
├── 6. Activity Log      — Last 30 audit entries
└── 7. Change My Password

Staff Menu
├── 1. Dashboard
├── 2. View Products
├── 3. Search Products
├── 4. Record Sale
├── 5. Restock Product
└── 6. Change My Password
```

## Data Structures

```cpp
struct Product {
    int id; string name, category, supplier;
    double price, costPrice;
    int quantity, lowStockThreshold;
    string expiryDate, addedDate; bool isActive;
};

struct SaleRecord {
    int saleId, productId; string productName;
    int quantitySold; double salePrice, totalRevenue;
    string saleDate, soldBy;
};

struct User {
    string username, passwordHash, lastLogin;
    Role role; bool isActive;
};
```

## Authors

- Harsh Kumar
