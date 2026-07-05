/*
  IMS Pro — Inventory Management Module
*/

const Inventory = (() => {
  let editMode = false;

  const getStatusBadge = (qty, threshold) => {
    if (qty <= 0) return `<span class="status-badge inactive">Out of Stock</span>`;
    if (qty <= threshold) return `<span class="status-badge low">Low Stock</span>`;
    return `<span class="status-badge active">In Stock</span>`;
  };

  return {
    init: () => {
      // Seed Category Dropdown in modal
      const select = document.getElementById("pf-category");
      select.innerHTML = "";
      Data.getCategories().forEach(cat => {
        select.innerHTML += `<option value="${cat}">${cat}</option>`;
      });

      // Hook up Form Submit
      document.getElementById("product-form").addEventListener("submit", Inventory.saveProduct);
      
      Inventory.render();
    },

    render: () => {
      const query = document.getElementById("inv-search").value.toLowerCase();
      const tbody = document.getElementById("inv-tbody");
      tbody.innerHTML = "";

      const products = Data.getProducts().filter(p => {
        return p.name.toLowerCase().includes(query) || 
               p.category.toLowerCase().includes(query) || 
               p.supplier.toLowerCase().includes(query) ||
               p.id.toString() === query;
      });

      document.getElementById("inv-count-label").textContent = `${products.length} product(s)`;

      if (products.length === 0) {
        tbody.innerHTML = `<tr><td colspan="10" style="text-align: center; color: var(--text-muted);">No products in inventory.</td></tr>`;
        return;
      }

      products.forEach(p => {
        const row = document.createElement("tr");
        const isLow = p.quantity <= p.lowStockThreshold;
        if (isLow) row.style.background = "rgba(245, 158, 11, 0.02)";
        
        row.innerHTML = `
          <td><strong>#${p.id}</strong></td>
          <td>${p.name}</td>
          <td><span class="category-pill sm" style="padding:4px 8px; font-size:11px;">${p.category}</span></td>
          <td>${p.supplier || 'N/A'}</td>
          <td>Rs. ${p.price.toFixed(2)}</td>
          <td>Rs. ${p.costPrice.toFixed(2)}</td>
          <td>
            <strong>${p.quantity}</strong> 
            ${getStatusBadge(p.quantity, p.lowStockThreshold)}
          </td>
          <td>${p.lowStockThreshold}</td>
          <td>${p.expiryDate || 'N/A'}</td>
          <td>
            <div style="display:flex; gap: 8px;">
              <button class="btn btn-ghost btn-sm" onclick="Inventory.openModal(${p.id})">✏️ Edit</button>
              <button class="btn btn-ghost btn-sm" style="color:var(--danger)" onclick="Inventory.delete(${p.id})">🗑 Delete</button>
            </div>
          </td>
        `;
        tbody.appendChild(row);
      });
    },

    openModal: (id = null) => {
      const modal = document.getElementById("product-modal");
      const form = document.getElementById("product-form");
      const title = document.getElementById("product-modal-title");

      form.reset();

      if (id) {
        editMode = true;
        title.textContent = "Edit Product";
        const prod = Data.getProductById(id);
        if (!prod) return;

        document.getElementById("pf-id").value = prod.id;
        document.getElementById("pf-name").value = prod.name;
        document.getElementById("pf-category").value = prod.category;
        document.getElementById("pf-supplier").value = prod.supplier;
        document.getElementById("pf-expiry").value = prod.expiryDate;
        document.getElementById("pf-price").value = prod.price;
        document.getElementById("pf-cost").value = prod.costPrice;
        document.getElementById("pf-qty").value = prod.quantity;
        document.getElementById("pf-threshold").value = prod.lowStockThreshold;
      } else {
        editMode = false;
        title.textContent = "Add New Product";
        document.getElementById("pf-id").value = "";
      }

      modal.classList.remove("hidden");
    },

    closeModal: () => {
      document.getElementById("product-modal").classList.add("hidden");
    },

    saveProduct: (e) => {
      e.preventDefault();
      
      const id = document.getElementById("pf-id").value;
      const productData = {
        name: document.getElementById("pf-name").value.trim(),
        category: document.getElementById("pf-category").value,
        supplier: document.getElementById("pf-supplier").value.trim(),
        expiryDate: document.getElementById("pf-expiry").value,
        price: parseFloat(document.getElementById("pf-price").value),
        costPrice: parseFloat(document.getElementById("pf-cost").value),
        quantity: parseInt(document.getElementById("pf-qty").value),
        lowStockThreshold: parseInt(document.getElementById("pf-threshold").value)
      };

      if (editMode && id) {
        Data.updateProduct(id, productData);
        App.showToast(`Updated product "${productData.name}"`, "success");
      } else {
        Data.addProduct(productData);
        App.showToast(`Added product "${productData.name}"`, "success");
      }

      Inventory.closeModal();
      Inventory.render();
      POS.renderProducts(); // Update POS page catalog as well
    },

    delete: (id) => {
      const prod = Data.getProductById(id);
      if (!prod) return;

      if (confirm(`Are you sure you want to delete "${prod.name}" from inventory?`)) {
        Data.deleteProduct(id);
        App.showToast(`Deleted "${prod.name}"`, "success");
        Inventory.render();
        POS.renderProducts();
      }
    }
  };
})();
