/*
  IMS Pro — Data Layer Module
  localStorage persistence & mock data seed helper
*/

const Data = (() => {
  // Categories
  const categories = [
    "Electronics", "Clothing", "Food & Beverages", 
    "Home & Garden", "Health & Beauty", "Sports & Outdoors", 
    "Books & Stationery", "Toys & Games", "Automotive", "Other"
  ];

  // Seed Products if localStorage is empty
  const defaultProducts = [
    { id: 1, name: "Sony WH-1000XM4", category: "Electronics", supplier: "Sony India", price: 19999.00, costPrice: 15000.00, quantity: 15, lowStockThreshold: 3, expiryDate: "", addedDate: "2026-06-01", isActive: true },
    { id: 2, name: "Nike Air Max 270", category: "Clothing", supplier: "Nike Retail", price: 11999.00, costPrice: 8500.00, quantity: 25, lowStockThreshold: 5, expiryDate: "", addedDate: "2026-06-02", isActive: true },
    { id: 3, name: "Pringles Sour Cream 165g", category: "Food & Beverages", supplier: "Kellogg Distributors", price: 110.00, costPrice: 75.00, quantity: 120, lowStockThreshold: 15, expiryDate: "2027-01-31", addedDate: "2026-06-05", isActive: true },
    { id: 4, name: "Dell MS116 Mouse", category: "Electronics", supplier: "Dell Corp", price: 349.00, costPrice: 200.00, quantity: 2, lowStockThreshold: 5, expiryDate: "", addedDate: "2026-06-05", isActive: true },
    { id: 5, name: "Casio G-Shock Matte Black", category: "Electronics", supplier: "Casio Retail", price: 7995.00, costPrice: 5000.00, quantity: 8, lowStockThreshold: 2, expiryDate: "", addedDate: "2026-06-08", isActive: true },
    { id: 6, name: "Milton Thermosteel 1L", category: "Home & Garden", supplier: "Milton Ltd", price: 1150.00, costPrice: 800.00, quantity: 30, lowStockThreshold: 6, expiryDate: "", addedDate: "2026-06-10", isActive: true },
    { id: 7, name: "Nivea Soft Cream 200ml", category: "Health & Beauty", supplier: "Beiersdorf", price: 299.00, costPrice: 190.00, quantity: 4, lowStockThreshold: 10, expiryDate: "2028-05-30", addedDate: "2026-06-12", isActive: true },
    { id: 8, name: "Parker Vector Rollerball", category: "Books & Stationery", supplier: "Luxor Pen Co", price: 300.00, costPrice: 180.00, quantity: 50, lowStockThreshold: 10, expiryDate: "", addedDate: "2026-06-14", isActive: true },
    { id: 9, name: "Monopoly Deal Card Game", category: "Toys & Games", supplier: "Hasbro India", price: 299.00, costPrice: 180.00, quantity: 40, lowStockThreshold: 8, expiryDate: "", addedDate: "2026-06-15", isActive: true },
    { id: 10, name: "Coca Cola Zero Can 330ml", category: "Food & Beverages", supplier: "Hindustan Coca Cola", price: 40.00, costPrice: 25.00, quantity: 200, lowStockThreshold: 25, expiryDate: "2026-12-15", addedDate: "2026-06-15", isActive: true },
    { id: 11, name: "Yuri Badminton Racket Set", category: "Sports & Outdoors", supplier: "Yuri Sports", price: 1450.00, costPrice: 900.00, quantity: 18, lowStockThreshold: 4, expiryDate: "", addedDate: "2026-06-18", isActive: true },
    { id: 12, name: "Red Bull Energy 250ml", category: "Food & Beverages", supplier: "Red Bull India", price: 125.00, costPrice: 90.00, quantity: 1, lowStockThreshold: 12, expiryDate: "2026-11-30", addedDate: "2026-06-20", isActive: true }
  ];

  // Seed Users
  const defaultUsers = [
    { username: "admin", passwordHash: "Admin@123", role: "admin", lastLogin: "2026-07-05 20:00:00", isActive: true },
    { username: "staff", passwordHash: "Staff@123", role: "staff", lastLogin: "2026-07-05 18:30:00", isActive: true }
  ];

  // Seed Sales
  const defaultSales = [
    { saleId: 1001, saleDate: "2026-06-29 10:15:30", items: [{ id: 3, name: "Pringles Sour Cream 165g", price: 110.00, quantity: 2 }, { id: 10, name: "Coca Cola Zero Can 330ml", price: 40.00, quantity: 4 }], subtotal: 380.00, discountPercent: 10, gst: 61.56, total: 403.56, paymentMethod: "cash", customerName: "Rahul Sharma", customerPhone: "9876543210", soldBy: "staff" },
    { saleId: 1002, saleDate: "2026-07-02 14:45:10", items: [{ id: 1, name: "Sony WH-1000XM4", price: 19999.00, quantity: 1 }], subtotal: 19999.00, discountPercent: 0, gst: 3599.82, total: 23598.82, paymentMethod: "card", customerName: "Amit Verma", customerPhone: "", soldBy: "admin" },
    { saleId: 1003, saleDate: "2026-07-04 18:20:00", items: [{ id: 6, name: "Milton Thermosteel 1L", price: 1150.00, quantity: 2 }, { id: 8, name: "Parker Vector Rollerball", price: 300.00, quantity: 1 }], subtotal: 2600.00, discountPercent: 5, gst: 444.60, total: 2914.60, paymentMethod: "upi", customerName: "Sneha Patel", customerPhone: "9988776655", soldBy: "staff" },
    { saleId: 1004, saleDate: "2026-07-05 11:30:15", items: [{ id: 2, name: "Nike Air Max 270", price: 11999.00, quantity: 1 }, { id: 10, name: "Coca Cola Zero Can 330ml", price: 40.00, quantity: 2 }], subtotal: 12079.00, discountPercent: 15, gst: 1848.09, total: 12115.24, paymentMethod: "upi", customerName: "Rohan Gupta", customerPhone: "", soldBy: "staff" }
  ];

  // Helper getters/setters for localStorage
  const get = (key, defaultVal) => {
    const val = localStorage.getItem(key);
    return val ? JSON.parse(val) : defaultVal;
  };
  const set = (key, val) => localStorage.setItem(key, JSON.stringify(val));

  // Initialize data if not existing
  const init = () => {
    if (!localStorage.getItem("ims_products")) set("ims_products", defaultProducts);
    if (!localStorage.getItem("ims_users")) set("ims_users", defaultUsers);
    if (!localStorage.getItem("ims_sales")) set("ims_sales", defaultSales);
  };

  init();

  return {
    getCategories: () => categories,
    
    // Products
    getProducts: () => get("ims_products", []).filter(p => p.isActive),
    getAllProductsRaw: () => get("ims_products", []),
    saveProducts: (products) => set("ims_products", products),
    
    getProductById: (id) => get("ims_products", []).find(p => p.id === parseInt(id) && p.isActive),
    
    addProduct: (prod) => {
      const prods = get("ims_products", []);
      const newId = prods.length > 0 ? Math.max(...prods.map(p => p.id)) + 1 : 1;
      const newProd = { ...prod, id: newId, isActive: true, addedDate: new Date().toISOString().split('T')[0] };
      prods.push(newProd);
      set("ims_products", prods);
      return newProd;
    },
    
    updateProduct: (id, updatedFields) => {
      const prods = get("ims_products", []);
      const index = prods.findIndex(p => p.id === parseInt(id));
      if (index !== -1) {
        prods[index] = { ...prods[index], ...updatedFields };
        set("ims_products", prods);
        return true;
      }
      return false;
    },
    
    deleteProduct: (id) => {
      // Soft delete
      return Data.updateProduct(id, { isActive: false });
    },

    // Sales
    getSales: () => get("ims_sales", []),
    addSale: (sale) => {
      const sales = get("ims_sales", []);
      const nextSaleId = sales.length > 0 ? Math.max(...sales.map(s => s.saleId)) + 1 : 1001;
      const newSale = { 
        ...sale, 
        saleId: nextSaleId, 
        saleDate: new Date().toISOString().replace('T', ' ').substring(0, 19) 
      };
      
      // Update quantities in inventory
      const prods = get("ims_products", []);
      for (const item of sale.items) {
        const prod = prods.find(p => p.id === item.id);
        if (prod) {
          prod.quantity = Math.max(0, prod.quantity - item.quantity);
        }
      }
      set("ims_products", prods);

      sales.push(newSale);
      set("ims_sales", sales);
      return newSale;
    },

    // Users
    getUsers: () => get("ims_users", []),
    saveUsers: (users) => set("ims_users", users),
    addUser: (user) => {
      const users = get("ims_users", []);
      if (users.find(u => u.username.toLowerCase() === user.username.toLowerCase())) {
        return false;
      }
      users.push({ ...user, lastLogin: "Never", isActive: true });
      set("ims_users", users);
      return true;
    },
    updateUser: (username, updatedFields) => {
      const users = get("ims_users", []);
      const index = users.findIndex(u => u.username.toLowerCase() === username.toLowerCase());
      if (index !== -1) {
        users[index] = { ...users[index], ...updatedFields };
        set("ims_users", users);
        return true;
      }
      return false;
    },
    deleteUser: (username) => {
      const users = get("ims_users", []);
      const filtered = users.filter(u => u.username.toLowerCase() !== username.toLowerCase());
      set("ims_users", filtered);
      return true;
    }
  };
})();
