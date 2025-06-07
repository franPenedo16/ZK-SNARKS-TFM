<template>
  <v-container class="fill-height" max-width="900">
    <v-card class="mx-auto" width="400">

      <template v-slot:title>
        <div class="text-center">
          <span class="font-weight-black">ZK-Snark Sudoku APP</span>
        </div>
      </template>

      <v-card-text class="bg-surface-light pt-4">
        <v-form>
          <v-container>
            <v-col>
              <v-text-field v-model="dni" label="DNI"></v-text-field>
            </v-col>
            <v-col>
              <v-text-field :append-inner-icon="show ? 'mdi-eye' : 'mdi-eye-off'" v-model="password"
                :type="show ? 'text' : 'password'" label="Password" @click:append-inner="show = !show"></v-text-field>
            </v-col>
          </v-container>
        </v-form>
      </v-card-text>
      <template v-slot:actions>
        <div class="mx-auto">
          <v-btn text="Login" color="gray" class="border" @click="login()"></v-btn>
        </div>
      </template>
    </v-card>

  </v-container>
  <v-snackbar v-model="notify['active']" location="top right" class="mt-16 cursor-pointer" :color="notify.color"
    @click="notify.active = false">
    <v-icon :icon="notify.icon"></v-icon> <span>{{ notify.text }}</span>
  </v-snackbar>
</template>

<script>
import { groth16 } from 'snarkjs'
import { useAppStore } from '../stores/app'
export default {
  setup() {
    const state = useAppStore()
    return { state }
  },
  async mounted() {
    try {
      const responseWasm = await fetch('/circuits/auth/circuit.wasm');
      const wasm = await responseWasm.arrayBuffer();
      const responseZkey = await fetch('/circuits/auth/circuit_0001.zkey');
      this.zkey = await responseZkey.arrayBuffer();
      this.wc = await this.getWitnessCalculator(wasm);

    } catch (error) {
      console.error('Error loading files WASM & ZKEY: ', error);
    }
  },
  data: () => ({
    dni: '',
    password: '',
    show: false,
    wc: null,
    zkey: null,
    notify: {
      active: false,
      text: '',
      color: '',
      icon: ''
    }
  }),
  methods: {
    async getWitnessCalculator(wasmBuffer) {
      const resp = await fetch('/circuits/witness_calculator.js');
      const jsCode = await resp.text();

      const module = {};
      const exports = {};
      const fn = new Function('module', 'exports', jsCode);
      fn(module, exports);

      return await module.exports(wasmBuffer);
    },
    toBase64(obj) {
      const json = JSON.stringify(obj);
      return btoa(json);
    },
    textToAscii(text) {
      const ascii = [];
      for (let i = 0; i < text.length; i++) {
        ascii.push(text.charCodeAt(i));
      }
      return ascii;
    },
    dniToArray(dni) {
      const result = [];
      for (let i = 0; i < dni.length; i++) {
        const char = dni[i];
        if (!isNaN(char)) {
          result.push(parseInt(char));
        } else {
          result.push(char.charCodeAt(0));
        }
      }
      return result;
    },
    async getUser() {
      const response = await fetch(`http://localhost:3500/api/user?dni=${this.dni}`, {
        method: 'GET',
        headers: {
          'Content-Type': 'application/json'
        },
      });
      if (!response.ok) {
        console.error('Error for api/user:', await response.text());
      } else {
        const res = await response.json();
        return res
      }
    },
    async login() {
      const userData = await this.getUser()
      const now = Math.floor(Date.now() / 1000);
      try {
        const input = {
          pass: this.textToAscii(this.password).join(''),
          dni: this.dniToArray(this.dni),
          hashBD: userData.hash,
          controlDate: now.toString(),
        };
        const witnessBin = await this.wc.calculateWTNSBin(input, true);


        const { proof, publicSignals } = await groth16.prove(
          new Uint8Array(this.zkey),
          witnessBin
        );

        const response = await fetch('http://localhost:3500/api/login', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify({
            proof_data: this.toBase64(proof),
            public_data: this.toBase64(publicSignals),
            dni: this.dni
          })
        });
        if (!response.ok) {
          console.error('Error sending proof:', await response.text());
          this.notify = {
            active: true,
            text: "There is an error sending the authentication proof.",
            color: "red-lighten-1",
            icon: "mdi-alert-circle"
          }
        } else {
          response.json().then(res => {
            const token = res.token;
            localStorage.setItem('token', token)
            this.state.initUserName();
            this.$router.push('/')
          })
        }
      }
      catch {
        this.notify = {
          active: true,
          text: "Wrong credentials",
          color: "red-lighten-1",
          icon: "mdi-alert-circle"
        }
      }
    }
  }
}
</script>
