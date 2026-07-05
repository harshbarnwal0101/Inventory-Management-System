/*
  IMS Pro — Dashboard and Analytics module
  Deals with loading Chart.js visualizations
*/

const Reports = (() => {
  let revenueChart = null;
  let categoryChart = null;
  let monthlyChart = null;
  let topProdChart = null;

  const getStats = () => {
    const products = Data.getProducts();
    const sales = Data.getSales();

    const totalProducts = products.length;
    const totalUnits = products.reduce((sum, p) => sum + p.quantity, 0);
    const stockValue = products.reduce((sum, p) => sum + (p.costPrice * p.quantity), 0);
    const saleValue = products.reduce((sum, p) => sum + (p.price * p.quantity), 0);
    const totalRevenue = sales.reduce((sum, s) => sum + s.total, 0);
    const totalUnitsSold = sales.reduce((sum, s) => sum + s.items.reduce((su, it) => su + it.quantity, 0), 0);

    return {
      totalProducts,
      totalUnits,
      stockValue,
      saleValue,
      totalRevenue,
      totalUnitsSold
    };
  };

  const getRecent7DaysRevenue = () => {
    const sales = Data.getSales();
    const revenueMap = {};

    // Seed last 7 days
    for (let i = 6; i >= 0; i--) {
      const d = new Date();
      d.setDate(d.getDate() - i);
      const dateStr = d.toISOString().split('T')[0];
      revenueMap[dateStr] = 0;
    }

    sales.forEach(s => {
      const dateStr = s.saleDate.split(' ')[0];
      if (revenueMap[dateStr] !== undefined) {
        revenueMap[dateStr] += s.total;
      }
    });

    return {
      labels: Object.keys(revenueMap).map(k => {
        const parts = k.split('-');
        return `${parts[2]}/${parts[1]}`; // DD/MM format
      }),
      data: Object.values(revenueMap)
    };
  };

  const getCategorySales = () => {
    const sales = Data.getSales();
    const products = Data.getAllProductsRaw();
    const catMap = {};

    Data.getCategories().forEach(c => catMap[c] = 0);

    sales.forEach(s => {
      s.items.forEach(item => {
        const prod = products.find(p => p.id === item.id);
        const cat = prod ? prod.category : "Other";
        catMap[cat] = (catMap[cat] || 0) + (item.price * item.quantity);
      });
    });

    // filter categories with sales > 0 to look cleaner
    const activeCats = Object.keys(catMap).filter(k => catMap[k] > 0);
    return {
      labels: activeCats,
      data: activeCats.map(k => catMap[k])
    };
  };

  const getTopProductsData = (n = 8) => {
    const sales = Data.getSales();
    const prodMap = {};

    sales.forEach(s => {
      s.items.forEach(item => {
        if (!prodMap[item.name]) {
          prodMap[item.name] = { name: item.name, quantity: 0, revenue: 0 };
        }
        prodMap[item.name].quantity += item.quantity;
        prodMap[item.name].revenue += item.price * item.quantity;
      });
    });

    const sorted = Object.values(prodMap).sort((a, b) => b.quantity - a.quantity).slice(0, n);
    return {
      labels: sorted.map(x => x.name),
      data: sorted.map(x => x.quantity),
      revenueData: sorted.map(x => x.revenue)
    };
  };

  const getMonthlyRevenue = () => {
    const sales = Data.getSales();
    const months = {};

    sales.forEach(s => {
      const monthStr = s.saleDate.substring(0, 7); // YYYY-MM
      months[monthStr] = (months[monthStr] || 0) + s.total;
    });

    const sortedMonths = Object.keys(months).sort();
    return {
      labels: sortedMonths.map(m => {
        const parts = m.split('-');
        const dateObj = new Date(parts[0], parseInt(parts[1]) - 1);
        return dateObj.toLocaleString('en-US', { month: 'short', year: '2-digit' });
      }),
      data: sortedMonths.map(m => months[m])
    };
  };

  const renderDashboardStats = () => {
    const stats = getStats();
    const container = document.getElementById("stats-grid");
    
    const formattedRevenue = stats.totalRevenue.toLocaleString('en-IN', { maximumFractionDigits: 2 });
    const formattedStock = stats.stockValue.toLocaleString('en-IN', { maximumFractionDigits: 2 });
    const formattedSale = stats.saleValue.toLocaleString('en-IN', { maximumFractionDigits: 2 });

    container.innerHTML = `
      <div class="stat-box" style="border-left: 4px solid var(--accent)">
        <span class="stat-label">Total Products</span>
        <span class="stat-value">${stats.totalProducts}</span>
      </div>
      <div class="stat-box" style="border-left: 4px solid var(--info)">
        <span class="stat-label">Total Stock Units</span>
        <span class="stat-value">${stats.totalUnits}</span>
      </div>
      <div class="stat-box" style="border-left: 4px solid var(--warning)">
        <span class="stat-label">Inventory Cost</span>
        <span class="stat-value">Rs. ${formattedStock}</span>
      </div>
      <div class="stat-box" style="border-left: 4px solid var(--success)">
        <span class="stat-label">Total Revenue</span>
        <span class="stat-value">Rs. ${formattedRevenue}</span>
      </div>
      <div class="stat-box" style="border-left: 4px solid var(--danger)">
        <span class="stat-label">Units Sold</span>
        <span class="stat-value">${stats.totalUnitsSold}</span>
      </div>
    `;

    // Low stock list
    const lowList = document.getElementById("low-stock-list");
    lowList.innerHTML = "";
    const lowStockProds = Data.getProducts().filter(p => p.quantity <= p.lowStockThreshold);

    if (lowStockProds.length === 0) {
      lowList.innerHTML = `<div style="text-align:center; color:var(--text-muted); padding: 20px;">All products adequately stocked.</div>`;
    } else {
      lowStockProds.forEach(p => {
        lowList.innerHTML += `
          <div class="low-stock-item">
            <div>
              <strong>${p.name}</strong>
              <div style="font-size:11px; color:var(--text-muted); margin-top:2px;">Category: ${p.category}</div>
            </div>
            <span class="alert-pill">${p.quantity <= 0 ? 'Out of Stock' : `Qty: ${p.quantity} (Limit: ${p.lowStockThreshold})`}</span>
          </div>
        `;
      });
    }

    // Top selling list (dashboard panel)
    const topList = document.getElementById("top-products-list");
    topList.innerHTML = "";
    const topProd = getTopProductsData(5);

    if (topProd.labels.length === 0) {
      topList.innerHTML = `<div style="text-align:center; color:var(--text-muted); padding: 20px;">No sales data available yet.</div>`;
    } else {
      topProd.labels.forEach((label, idx) => {
        topList.innerHTML += `
          <div class="top-item">
            <span><strong>${idx + 1}. ${label}</strong></span>
            <span style="font-weight:700; color:var(--accent);">${topProd.data[idx]} units sold</span>
          </div>
        `;
      });
    }
  };

  const renderDashboardCharts = () => {
    // 7-day Revenue Line Chart
    const revData = getRecent7DaysRevenue();
    const ctx1 = document.getElementById("chart-revenue").getContext("2d");

    if (revenueChart) revenueChart.destroy();
    revenueChart = new Chart(ctx1, {
      type: 'line',
      data: {
        labels: revData.labels,
        datasets: [{
          label: 'Revenue (Rs.)',
          data: revData.data,
          borderColor: '#6366f1',
          backgroundColor: 'rgba(99, 102, 241, 0.1)',
          fill: true,
          tension: 0.3,
          borderWidth: 3
        }]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: { legend: { display: false } },
        scales: {
          y: { grid: { color: 'rgba(255,255,255,0.05)' }, ticks: { color: '#9ca3af' } },
          x: { grid: { display: false }, ticks: { color: '#9ca3af' } }
        }
      }
    });

    // Category Doughnut Chart
    const catData = getCategorySales();
    const ctx2 = document.getElementById("chart-category").getContext("2d");

    if (categoryChart) categoryChart.destroy();
    categoryChart = new Chart(ctx2, {
      type: 'doughnut',
      data: {
        labels: catData.labels,
        datasets: [{
          data: catData.data,
          backgroundColor: [
            '#6366f1', '#10b981', '#f59e0b', '#ef4444', 
            '#06b6d4', '#ec4899', '#8b5cf6', '#3b82f6'
          ],
          borderWidth: 0
        }]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
          legend: {
            position: 'right',
            labels: { color: '#9ca3af', boxWidth: 12, padding: 12 }
          }
        }
      }
    });
  };

  const renderReportsPage = () => {
    const stats = getStats();
    const container = document.getElementById("reports-stats");
    
    const formattedRevenue = stats.totalRevenue.toLocaleString('en-IN', { maximumFractionDigits: 2 });
    const profit = Data.getSales().reduce((sum, s) => {
      // Calculate profit: revenue - item cost price
      const sProfit = s.items.reduce((sSum, item) => {
        const origProd = Data.getAllProductsRaw().find(p => p.id === item.id);
        const cost = origProd ? origProd.costPrice : item.price * 0.7; // fallback
        return sSum + ((item.price - cost) * item.quantity);
      }, 0);
      return sum + (sProfit - (s.subtotal * (s.discountPercent / 100)));
    }, 0);
    const formattedProfit = profit.toLocaleString('en-IN', { maximumFractionDigits: 2 });

    container.innerHTML = `
      <div class="stats-grid">
        <div class="stat-box" style="border-left: 4px solid var(--success)">
          <span class="stat-label">Gross Revenue</span>
          <span class="stat-value">Rs. ${formattedRevenue}</span>
        </div>
        <div class="stat-box" style="border-left: 4px solid var(--info)">
          <span class="stat-label">Net Sales Profit</span>
          <span class="stat-value">Rs. ${formattedProfit}</span>
        </div>
        <div class="stat-box" style="border-left: 4px solid var(--accent)">
          <span class="stat-label">Transaction Count</span>
          <span class="stat-value">${Data.getSales().length}</span>
        </div>
      </div>
    `;

    // Monthly Revenue Bar Chart
    const monData = getMonthlyRevenue();
    const ctx1 = document.getElementById("chart-monthly").getContext("2d");

    if (monthlyChart) monthlyChart.destroy();
    monthlyChart = new Chart(ctx1, {
      type: 'bar',
      data: {
        labels: monData.labels,
        datasets: [{
          label: 'Revenue (Rs.)',
          data: monData.data,
          backgroundColor: '#10b981',
          borderRadius: 6
        }]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: { legend: { display: false } },
        scales: {
          y: { grid: { color: 'rgba(255,255,255,0.05)' }, ticks: { color: '#9ca3af' } },
          x: { grid: { display: false }, ticks: { color: '#9ca3af' } }
        }
      }
    });

    // Top Products Horizontal Bar Chart
    const topData = getTopProductsData(8);
    const ctx2 = document.getElementById("chart-top-prod").getContext("2d");

    if (topProdChart) topProdChart.destroy();
    topProdChart = new Chart(ctx2, {
      type: 'bar',
      data: {
        labels: topData.labels,
        datasets: [{
          label: 'Units Sold',
          data: topData.data,
          backgroundColor: '#6366f1',
          borderRadius: 6
        }]
      },
      options: {
        indexAxis: 'y',
        responsive: true,
        maintainAspectRatio: false,
        plugins: { legend: { display: false } },
        scales: {
          x: { grid: { color: 'rgba(255,255,255,0.05)' }, ticks: { color: '#9ca3af' } },
          y: { grid: { display: false }, ticks: { color: '#9ca3af' } }
        }
      }
    });
  };

  return {
    renderDashboard: () => {
      const now = new Date();
      document.getElementById("dash-datetime").textContent = now.toLocaleString('en-US', { 
        weekday: 'long', year: 'numeric', month: 'long', day: 'numeric', hour: '2-digit', minute: '2-digit' 
      });

      renderDashboardStats();
      renderDashboardCharts();
    },

    renderReports: () => {
      renderReportsPage();
    }
  };
})();
