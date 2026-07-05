/*
  IMS Pro — App Orchestrator & router
*/

const App = (() => {
  let toastTimer = null;

  const handleNavClick = (e) => {
    e.preventDefault();
    const pageId = e.currentTarget.getAttribute("data-page");
    App.navigate(pageId);
  };

  const checkAuth = () => {
    const loginScreen = document.getElementById("login-screen");
    const mainApp = document.getElementById("app");

    if (Auth.isAuthenticated()) {
      loginScreen.classList.add("hidden");
      mainApp.classList.remove("hidden");

      // Setup user details in sidebar
      const user = Auth.getCurrentUser();
      document.getElementById("user-name").textContent = user.username;
      document.getElementById("user-role").textContent = user.role === "admin" ? "Administrator" : "Billing Staff";
      document.getElementById("user-avatar").textContent = user.username.substring(0, 1).toUpperCase();

      // Show/hide admin-only nav options
      document.querySelectorAll(".admin-only").forEach(el => {
        if (user.role === "admin") {
          el.classList.remove("hidden");
        } else {
          el.classList.add("hidden");
        }
      });

      // Default page navigation
      App.navigate("dashboard");
    } else {
      loginScreen.classList.remove("hidden");
      mainApp.classList.add("hidden");
    }
  };

  return {
    init: () => {
      // Setup sidebar nav clicks
      document.querySelectorAll(".nav-item").forEach(item => {
        item.addEventListener("click", handleNavClick);
      });

      // Setup logout button
      document.getElementById("logout-btn").addEventListener("click", () => {
        Auth.logout();
        App.showToast("Logged out successfully.", "info");
        checkAuth();
      });

      // Setup login form submission
      document.getElementById("login-form").addEventListener("submit", (e) => {
        e.preventDefault();
        const u = document.getElementById("login-username").value.trim();
        const p = document.getElementById("login-password").value;
        const err = document.getElementById("login-error");

        if (Auth.login(u, p)) {
          err.classList.add("hidden");
          document.getElementById("login-form").reset();
          App.showToast(`Welcome back, ${u}!`, "success");
          checkAuth();
        } else {
          err.textContent = "Invalid username or password";
          err.classList.remove("hidden");
        }
      });

      // Setup password visibility toggle
      document.getElementById("toggle-pwd").addEventListener("click", () => {
        const input = document.getElementById("login-password");
        if (input.type === "password") {
          input.type = "text";
        } else {
          input.type = "password";
        }
      });

      // Initialize sub-modules
      POS.init();
      Inventory.init();

      // Perform initial auth check
      checkAuth();
    },

    navigate: (pageId) => {
      // Check roles
      if (pageId === "users" && !Auth.isAdmin()) {
        App.showToast("🚫 Access Denied: Admin role required", "danger");
        return;
      }

      // Toggle active link
      document.querySelectorAll(".nav-item").forEach(item => {
        if (item.getAttribute("data-page") === pageId) {
          item.classList.add("active");
        } else {
          item.classList.remove("active");
        }
      });

      // Toggle visible page
      document.querySelectorAll("main section").forEach(section => {
        if (section.id === `page-${pageId}`) {
          section.classList.add("active");
        } else {
          section.classList.remove("active");
        }
      });

      // Page-specific initialization/refresh triggers
      switch (pageId) {
        case "dashboard":
          Reports.renderDashboard();
          break;
        case "pos":
          POS.renderProducts();
          break;
        case "inventory":
          Inventory.render();
          break;
        case "sales":
          Sales.render();
          break;
        case "reports":
          Reports.renderReports();
          break;
        case "users":
          Users.render();
          break;
      }
    },

    showToast: (msg, type = "success") => {
      const toast = document.getElementById("toast");
      clearTimeout(toastTimer);

      let icon = "✔️";
      if (type === "warning") icon = "⚠️";
      if (type === "danger") icon = "🚫";
      if (type === "info") icon = "ℹ️";

      toast.innerHTML = `<span>${icon}</span> <span>${msg}</span>`;
      
      // color customization
      toast.style.borderColor = `var(--${type})`;
      toast.style.boxShadow = `0 10px 30px -10px var(--${type}-glow)`;

      toast.classList.remove("hidden");

      toastTimer = setTimeout(() => {
        toast.classList.add("hidden");
      }, 3500);
    }
  };
})();

// ── SALES VIEW PAGE ORCHESTRATION ──────────────────────────────────────────
const Sales = (() => {
  return {
    clearFilter: () => {
      document.getElementById("sales-date-filter").value = "";
      Sales.render();
    },

    render: () => {
      const filterDate = document.getElementById("sales-date-filter").value;
      const tbody = document.getElementById("sales-tbody");
      tbody.innerHTML = "";

      let sales = Data.getSales();
      if (filterDate) {
        sales = sales.filter(s => s.saleDate.startsWith(filterDate));
      }

      // Sort descending (newest first)
      sales.sort((a, b) => b.saleId - a.saleId);

      if (sales.length === 0) {
        tbody.innerHTML = `<tr><td colspan="8" style="text-align: center; color: var(--text-muted);">No sales records found.</td></tr>`;
        return;
      }

      sales.forEach(s => {
        const row = document.createElement("tr");
        const itemsList = s.items.map(it => `${it.name} (x${it.quantity})`).join(", ");
        
        row.innerHTML = `
          <td><strong>TX-${s.saleId}</strong></td>
          <td>${s.saleDate}</td>
          <td style="max-width: 250px; overflow: hidden; text-overflow: ellipsis; white-space: nowrap;" title="${itemsList}">${itemsList}</td>
          <td>${s.customerName || 'Walk-in'} ${s.customerPhone ? `(${s.customerPhone})` : ''}</td>
          <td><span class="category-pill sm" style="background: rgba(255,255,255,0.05);">${s.paymentMethod.toUpperCase()}</span></td>
          <td><strong>Rs. ${s.total.toFixed(2)}</strong></td>
          <td><code>${s.soldBy}</code></td>
          <td><button class="btn btn-ghost btn-sm" onclick="POS.showReceipt(${JSON.stringify(s).replace(/"/g, '&quot;')})">📄 View</button></td>
        `;
        tbody.appendChild(row);
      });
    }
  };
})();

// ── USER MANAGEMENT PAGE ORCHESTRATION ──────────────────────────────────────
const Users = (() => {
  let editMode = false;

  return {
    init: () => {
      document.getElementById("user-form").addEventListener("submit", Users.saveUser);
    },

    render: () => {
      const tbody = document.getElementById("users-tbody");
      tbody.innerHTML = "";

      const users = Data.getUsers();
      users.forEach(u => {
        const row = document.createElement("tr");
        const isCurrent = u.username.toLowerCase() === Auth.getCurrentUser().username.toLowerCase();

        row.innerHTML = `
          <td><strong>${u.username}</strong> ${isCurrent ? '<span style="font-size:10px; opacity:0.5;">(You)</span>' : ''}</td>
          <td><span class="category-pill sm" style="background:${u.role === 'admin' ? 'var(--accent-glow)' : 'rgba(255,255,255,0.05)'}; color:${u.role === 'admin' ? 'var(--accent)' : 'inherit'};">${u.role.toUpperCase()}</span></td>
          <td>${u.lastLogin || 'Never'}</td>
          <td>${u.isActive ? '<span class="status-badge active">Active</span>' : '<span class="status-badge inactive">Disabled</span>'}</td>
          <td>
            <div style="display:flex; gap: 8px;">
              <button class="btn btn-ghost btn-sm" onclick="Users.openModal('${u.username}')">✏️ Edit</button>
              ${!isCurrent ? `<button class="btn btn-ghost btn-sm" style="color:var(--danger)" onclick="Users.delete('${u.username}')">🗑 Delete</button>` : ''}
            </div>
          </td>
        `;
        tbody.appendChild(row);
      });
    },

    openModal: (username = null) => {
      const modal = document.getElementById("user-modal");
      const form = document.getElementById("user-form");
      const title = document.getElementById("user-modal-title");

      form.reset();
      document.getElementById("uf-username").disabled = false;

      if (username) {
        editMode = true;
        title.textContent = "Edit User Settings";
        const users = Data.getUsers();
        const user = users.find(u => u.username.toLowerCase() === username.toLowerCase());
        if (!user) return;

        document.getElementById("uf-id").value = user.username;
        document.getElementById("uf-username").value = user.username;
        document.getElementById("uf-username").disabled = true; // cannot rename key username
        document.getElementById("uf-role").value = user.role;
        document.getElementById("uf-password").placeholder = "Leave blank to keep current";
        document.getElementById("uf-password").required = false;
      } else {
        editMode = false;
        title.textContent = "Add New Staff User";
        document.getElementById("uf-id").value = "";
        document.getElementById("uf-password").placeholder = "Min 6 characters";
        document.getElementById("uf-password").required = true;
      }

      modal.classList.remove("hidden");
    },

    closeModal: () => {
      document.getElementById("user-modal").classList.add("hidden");
    },

    saveUser: (e) => {
      e.preventDefault();
      
      const id = document.getElementById("uf-id").value;
      const username = document.getElementById("uf-username").value.trim();
      const password = document.getElementById("uf-password").value;
      const role = document.getElementById("uf-role").value;

      if (editMode && id) {
        // Update user
        const updateFields = { role };
        if (password) {
          updateFields.passwordHash = password; // Stores plain/hash based on application logic
        }
        Data.updateUser(id, updateFields);
        App.showToast(`User settings for "${id}" updated`, "success");
      } else {
        // Add user
        const success = Data.addUser({
          username,
          passwordHash: password,
          role
        });

        if (success) {
          App.showToast(`Staff account "${username}" created!`, "success");
        } else {
          App.showToast("⚠️ Username already exists", "warning");
          return;
        }
      }

      Users.closeModal();
      Users.render();
    },

    delete: (username) => {
      if (confirm(`Are you sure you want to remove the user account "${username}"?`)) {
        Data.deleteUser(username);
        App.showToast(`User account "${username}" removed`, "success");
        Users.render();
      }
    }
  };
})();

// Initialize User management handlers
window.addEventListener("DOMContentLoaded", () => {
  Users.init();
  App.init();
});
