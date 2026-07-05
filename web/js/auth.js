/*
  IMS Pro — Authentication Module
*/

const Auth = (() => {
  let currentUser = null;

  return {
    getCurrentUser: () => {
      if (!currentUser) {
        const stored = sessionStorage.getItem("ims_current_user");
        if (stored) {
          currentUser = JSON.parse(stored);
        }
      }
      return currentUser;
    },

    login: (username, password) => {
      const users = Data.getUsers();
      const user = users.find(u => 
        u.username.toLowerCase() === username.toLowerCase() && 
        u.passwordHash === password && 
        u.isActive
      );

      if (user) {
        // Record last login
        const nowStr = new Date().toISOString().replace('T', ' ').substring(0, 19);
        Data.updateUser(user.username, { lastLogin: nowStr });
        
        currentUser = {
          username: user.username,
          role: user.role
        };
        sessionStorage.setItem("ims_current_user", JSON.stringify(currentUser));
        return true;
      }
      return false;
    },

    logout: () => {
      currentUser = null;
      sessionStorage.removeItem("ims_current_user");
    },

    isAdmin: () => {
      const user = Auth.getCurrentUser();
      return user && user.role === "admin";
    },

    isAuthenticated: () => {
      return Auth.getCurrentUser() !== null;
    }
  };
})();
