// Utilities
import { defineStore } from 'pinia'
import { jwtDecode } from 'jwt-decode'

interface MyJwtPayload {
  id: string
  dni: string
  username: string
}

export const useAppStore = defineStore('app', {
  state: () => ({
    user: {
      id: '', 
      dni: '', 
      name: '' 
    },
  }),
  actions: {
    initUserName() {
      let username = '';
      let id = '';
      let dni = '';
      if (this.user.id === '') {
        const token = localStorage.getItem('token');
        try {
          if (token) {
            const decoded = jwtDecode<MyJwtPayload>(token);
            id = decoded.id;
            username = decoded.username;
            dni = decoded.dni;
          }
        } catch (error) {
          console.error(error);
        }
        this.user.id = id;
        this.user.name = username;
        this.user.dni = dni;
      }
    }
  }
})

