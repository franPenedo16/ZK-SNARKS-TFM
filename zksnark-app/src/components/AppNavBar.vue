<template>
  <v-system-bar window class="pa-5">
    <router-link to="/" class="text-black text-decoration-none">
      <h2><b>ZK-Snark Sudoku APP</b></h2>
    </router-link>

    <v-icon class="ml-10" icon="mdi-account-circle"></v-icon>
    <span class="text-subtitle-2"><b>&nbsp; {{ state.user.name }}</b></span>
    <v-spacer></v-spacer>

    <v-btn prepend-icon="mdi-vector-polyline" variant="elevated" size="small" color="#A5D6A7" class="mr-10"
      to="/verification">
      Verifications
    </v-btn>
    <v-btn density="compact" icon="mdi-close-circle" color="#B71C1C" @click="logout()"></v-btn>
  </v-system-bar>
</template>

<script>
import { useAppStore } from '../stores/app'
export default {
  setup() {
    const state = useAppStore();
    return { state }
  },
  async mounted() {
    this.state.initUserName();
  },
  data: () => ({}),
  methods: {
    logout() {
      localStorage.removeItem('token')
      this.$router.push('/login')
    }
  }
}
</script>