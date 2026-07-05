/*
  IMS Pro — POS Billing Module
*/

const POS = (() => {
  let cart = [];
  let selectedCategory = "All";
  let activeBillNo = "";

  const getGST = (subtotal) => parseFloat((subtotal * 0.18).toFixed(2));

  const updateCartSummary = () => {
    const subtotal = cart.reduce((sum, item) => sum + (item.price * item.quantity), 0);
    const discPct = parseInt(document.getElementById("discount-input").value) || 0;
    const discVal = parseFloat((subtotal * (discPct / 100)).toFixed(2));
    const gstVal = getGST(subtotal - discVal);
    const total = parseFloat((subtotal - discVal + gstVal).toFixed(2));

    document.getElementById("cart-subtotal").textContent = `Rs. ${subtotal.toLocaleString('en-IN', { minimumFractionDigits: 2, maximumFractionDigits: 2 })}`;
    document.getElementById("cart-discount-val").textContent = `- Rs. ${discVal.toLocaleString('en-IN', { minimumFractionDigits: 2, maximumFractionDigits: 2 })}`;
    document.getElementById("cart-gst").textContent = `Rs. ${gstVal.toLocaleString('en-IN', { minimumFractionDigits: 2, maximumFractionDigits: 2 })}`;
    document.getElementById("cart-total").textContent = `Rs. ${total.toLocaleString('en-IN', { minimumFractionDigits: 2, maximumFractionDigits: 2 })}`;

    // Update cart nav badge
    const badge = document.getElementById("cart-badge");
    const totalQty = cart.reduce((sum, item) => sum + item.quantity, 0);
    if (totalQty > 0) {
      badge.textContent = totalQty;
      badge.style.display = "inline-flex";
    } else {
      badge.style.display = "none";
    }
  };

  const renderCart = () => {
    const container = document.getElementById("cart-items");
    const emptyEl = document.getElementById("cart-empty");

    // Clear items except the empty element
    container.innerHTML = "";
    container.appendChild(emptyEl);

    if (cart.length === 0) {
      emptyEl.classList.remove("hidden");
      updateCartSummary();
      return;
    }

    emptyEl.classList.add("hidden");

    cart.forEach(item => {
      const itemEl = document.createElement("div");
      itemEl.className = "cart-item";
      itemEl.innerHTML = `
        <div class="cart-item-info">
          <div class="cart-item-name">${item.name}</div>
          <div class="cart-item-price">Rs. ${item.price.toFixed(2)}</div>
        </div>
        <div class="cart-item-qty-control">
          <button class="qty-btn" onclick="POS.changeQty(${item.id}, -1)">-</button>
          <span>${item.quantity}</span>
          <button class="qty-btn" onclick="POS.changeQty(${item.id}, 1)">+</button>
        </div>
        <div class="cart-item-total">Rs. ${(item.price * item.quantity).toFixed(2)}</div>
      `;
      container.appendChild(itemEl);
    });

    updateCartSummary();
  };

  return {
    init: () => {
      POS.clearCart();
      
      // Hook up discount changes
      document.getElementById("discount-input").addEventListener("input", updateCartSummary);
      
      // Hook up search filter
      document.getElementById("pos-search").addEventListener("input", POS.renderProducts);

      // Render category pills
      const catContainer = document.getElementById("pos-categories");
      catContainer.innerHTML = `<div class="category-pill active" onclick="POS.setCategory('All', this)">All Items</div>`;
      Data.getCategories().forEach(cat => {
        catContainer.innerHTML += `<div class="category-pill" onclick="POS.setCategory('${cat}', this)">${cat}</div>`;
      });
      
      POS.renderProducts();
    },

    setCategory: (cat, el) => {
      selectedCategory = cat;
      document.querySelectorAll(".category-pill").forEach(p => p.classList.remove("active"));
      el.classList.add("active");
      POS.renderProducts();
    },

    renderProducts: () => {
      const query = document.getElementById("pos-search").value.toLowerCase();
      const grid = document.getElementById("pos-product-grid");
      grid.innerHTML = "";

      const filtered = Data.getProducts().filter(p => {
        const matchesCat = selectedCategory === "All" || p.category === selectedCategory;
        const matchesQuery = p.name.toLowerCase().includes(query) || p.category.toLowerCase().includes(query) || p.id.toString() === query;
        return matchesCat && matchesQuery;
      });

      if (filtered.length === 0) {
        grid.innerHTML = `<div style="grid-column: 1/-1; text-align: center; color: var(--text-muted); padding: 40px 0;">No products found</div>`;
        return;
      }

      filtered.forEach(p => {
        const isOutOfStock = p.quantity <= 0;
        const isLowStock = p.quantity <= p.lowStockThreshold;
        
        const card = document.createElement("div");
        card.className = `prod-card ${isOutOfStock ? 'out-of-stock' : ''} ${isLowStock && !isOutOfStock ? 'low-stock' : ''}`;
        card.innerHTML = `
          <div class="prod-name">${p.name}</div>
          <div style="display: flex; justify-content: space-between; align-items: flex-end;">
            <div class="prod-price">Rs. ${p.price.toFixed(2)}</div>
            <div class="prod-stock-label">${isOutOfStock ? 'OUT OF STOCK' : `Stock: ${p.quantity}`}</div>
          </div>
        `;

        if (!isOutOfStock) {
          card.onclick = () => POS.addToCart(p.id);
        }
        grid.appendChild(card);
      });
    },

    addToCart: (id) => {
      const product = Data.getProductById(id);
      if (!product) return;

      const existingItem = cart.find(item => item.id === product.id);
      if (existingItem) {
        if (existingItem.quantity >= product.quantity) {
          App.showToast(`⚠️ Only ${product.quantity} units of ${product.name} are available in stock.`, "warning");
          return;
        }
        existingItem.quantity += 1;
      } else {
        cart.push({
          id: product.id,
          name: product.name,
          price: product.price,
          quantity: 1
        });
      }

      App.showToast(`Added ${product.name} to bill`, "success");
      renderCart();
    },

    changeQty: (id, amount) => {
      const item = cart.find(i => i.id === id);
      if (!item) return;

      const product = Data.getProductById(id);
      
      if (amount > 0 && item.quantity >= product.quantity) {
        App.showToast(`⚠️ Insufficient stock. Only ${product.quantity} units available.`, "warning");
        return;
      }

      item.quantity += amount;
      if (item.quantity <= 0) {
        cart = cart.filter(i => i.id !== id);
      }
      renderCart();
    },

    clearCart: () => {
      cart = [];
      document.getElementById("discount-input").value = 0;
      document.getElementById("customer-name").value = "";
      document.getElementById("customer-phone").value = "";
      
      const sales = Data.getSales();
      const nextSaleId = sales.length > 0 ? Math.max(...sales.map(s => s.saleId)) + 1 : 1001;
      activeBillNo = `TX-${nextSaleId}`;
      document.getElementById("cart-bill-no").textContent = `Bill #${activeBillNo}`;
      
      const now = new Date();
      document.getElementById("cart-date-time").textContent = now.toLocaleString();
      renderCart();
    },

    checkout: (paymentMethod) => {
      if (cart.length === 0) {
        App.showToast("⚠️ Cart is empty! Add products first.", "warning");
        return;
      }

      const subtotal = cart.reduce((sum, item) => sum + (item.price * item.quantity), 0);
      const discPct = parseInt(document.getElementById("discount-input").value) || 0;
      const discVal = parseFloat((subtotal * (discPct / 100)).toFixed(2));
      const gstVal = getGST(subtotal - discVal);
      const total = parseFloat((subtotal - discVal + gstVal).toFixed(2));

      const customerName = document.getElementById("customer-name").value.trim();
      const customerPhone = document.getElementById("customer-phone").value.trim();

      const user = Auth.getCurrentUser();

      const newSale = {
        items: [...cart],
        subtotal: subtotal,
        discountPercent: discPct,
        gst: gstVal,
        total: total,
        paymentMethod: paymentMethod,
        customerName: customerName,
        customerPhone: customerPhone,
        soldBy: user ? user.username : "unknown"
      };

      const recordedSale = Data.addSale(newSale);
      App.showToast(`Bill #${recordedSale.saleId} generated successfully!`, "success");
      
      // Render Receipt
      POS.showReceipt(recordedSale);
      
      // Clear Cart
      POS.clearCart();
      POS.renderProducts();
    },

    showReceipt: (sale) => {
      const container = document.getElementById("receipt-content");
      const user = Auth.getCurrentUser();
      
      const itemsHtml = sale.items.map(item => `
        <div class="receipt-row">
          <span>${item.name}</span>
          <span>Rs. ${(item.price * item.quantity).toFixed(2)}</span>
        </div>
        <div class="receipt-item-detail">
          ${item.quantity} x Rs. ${item.price.toFixed(2)}
        </div>
      `).join('');

      container.innerHTML = `
        <div class="receipt-center">
          <div class="receipt-title">RETAIL MART</div>
          <div>MALL ROAD, SECTOR-5</div>
          <div>DELHI - 110001</div>
          <div>PHONE: +91-9988776655</div>
        </div>
        <div class="receipt-dashed"></div>
        <div class="receipt-row">
          <span>DATE: ${sale.saleDate.split(' ')[0]}</span>
          <span>TIME: ${sale.saleDate.split(' ')[1]}</span>
        </div>
        <div class="receipt-row">
          <span>BILL NO: TX-${sale.saleId}</span>
          <span>CASHIER: ${sale.soldBy}</span>
        </div>
        <div class="receipt-row">
          <span>CUSTOMER: ${sale.customerName || 'Walk-in'}</span>
        </div>
        ${sale.customerPhone ? `<div class="receipt-row"><span>PHONE: ${sale.customerPhone}</span></div>` : ''}
        <div class="receipt-dashed"></div>
        <div class="receipt-items">
          ${itemsHtml}
        </div>
        <div class="receipt-dashed"></div>
        <div class="receipt-row">
          <span>SUBTOTAL</span>
          <span>Rs. ${sale.subtotal.toFixed(2)}</span>
        </div>
        ${sale.discountPercent > 0 ? `
        <div class="receipt-row">
          <span>DISCOUNT (${sale.discountPercent}%)</span>
          <span>-Rs. ${(sale.subtotal * (sale.discountPercent / 100)).toFixed(2)}</span>
        </div>` : ''}
        <div class="receipt-row">
          <span>GST (18%)</span>
          <span>Rs. ${sale.gst.toFixed(2)}</span>
        </div>
        <div class="receipt-dashed"></div>
        <div class="receipt-row" style="font-weight: 800; font-size:13px">
          <span>NET PAYABLE</span>
          <span>Rs. ${sale.total.toFixed(2)}</span>
        </div>
        <div class="receipt-dashed"></div>
        <div class="receipt-row">
          <span>PAID BY:</span>
          <span>${sale.paymentMethod.toUpperCase()}</span>
        </div>
        <div class="receipt-footer">
          <div style="font-weight: 600; margin-bottom: 5px;">THANK YOU FOR SHOPPING!</div>
          <div>VISIT AGAIN</div>
          <div style="margin-top: 10px; font-size: 8px; color: #444;">Powered by IMS Pro</div>
        </div>
      `;

      document.getElementById("receipt-modal").classList.remove("hidden");
    },

    closeReceipt: () => {
      document.getElementById("receipt-modal").classList.add("hidden");
    },

    printReceipt: () => {
      const receiptContent = document.getElementById("receipt-content").innerHTML;
      const printWindow = window.open('', '', 'height=600,width=400');
      printWindow.document.write('<html><head><title>Print Receipt</title>');
      printWindow.document.write('<style>');
      printWindow.document.write(`
        body { font-family: monospace; font-size: 12px; margin: 0; padding: 20px; color: #000; background: #fff; width: 300px; }
        .receipt-center { text-align: center; margin-bottom: 12px; }
        .receipt-title { font-weight: 800; font-size: 16px; }
        .receipt-dashed { border-top: 1px dashed #000; margin: 10px 0; }
        .receipt-row { display: flex; justify-content: space-between; }
        .receipt-items { margin: 10px 0; }
        .receipt-item-detail { padding-left: 10px; font-size: 11px; }
        .receipt-footer { margin-top: 20px; text-align: center; font-size: 10px; }
      `);
      printWindow.document.write('</style></head><body>');
      printWindow.document.write(receiptContent);
      printWindow.document.write('</body></html>');
      printWindow.document.close();
      printWindow.focus();
      setTimeout(() => {
        printWindow.print();
        printWindow.close();
      }, 250);
    }
  };
})();
