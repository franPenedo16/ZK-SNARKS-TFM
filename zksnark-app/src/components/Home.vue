<template>
  <AppNavBar />
  <v-container class="fill-height" max-width="900">
    <div class="d-flex h-100 w-100 mx-auto text-center flex-column">
      <h1 class="mt-10"> TODAY'S SUDOKU </h1>
      <h2 class="">{{ dateNow }}</h2>
      <v-card class="mx-auto mt-16 w-50" elevation="16">
        <v-card-item class="mb-5 bg-blue-grey">
          <v-btn text="Easy" color="green" class="border mr-5" :class="currentLevel === 'easy' ? 'selected' : ''"
            @click="getSudoku('easy')"></v-btn>
          <v-btn text="Medium" color="orange" class="border text-white mr-5"
            :class="currentLevel === 'medium' ? 'selected' : ''" @click="getSudoku('medium')"></v-btn>
          <v-btn text="Hard" color="red" class="border" :class="currentLevel === 'hard' ? 'selected' : ''"
            @click="getSudoku('hard')"></v-btn>
        </v-card-item>
        <v-card-text class="mt-6">
          <v-container class="pa-1" v-if="puzzle.length > 0">
            <v-item-group multiple>
              <v-row v-for="(array, i) in puzzle" :key="i">
                <v-col v-for="(number, k) in array" :class="k === 2 || k === 5 ? 'border-e' : ''">
                  <span v-if="number > 0">{{ number }}</span>
                  <div v-else-if="String(number).includes('b')" class="bg-grey-darken-1 no-caret"
                    :contenteditable="true" @beforeinput="(event) => handleBeforeInput(event, i, k)">
                    {{ String(number).replace('b', '') }}
                  </div>
                  <div v-else class="bg-grey-darken-1 no-caret" :contenteditable="true"
                    @beforeinput="(event) => handleBeforeInput(event, i, k)"> 0
                  </div>
                </v-col>
                <v-divider v-if="i === 2 || i === 5"></v-divider>
              </v-row>
            </v-item-group>
          </v-container>
          <v-skeleton-loader v-else type="card"></v-skeleton-loader>
        </v-card-text>
        <v-card-actions>
          <v-btn variant="flat" text="Create ZK proof" block color="#A5D6A7" class="border"
            @click="dialog = true"></v-btn>
        </v-card-actions>
      </v-card>
    </div>
  </v-container>
  <v-dialog v-model="dialog" width="auto" persistent>
    <v-card v-if="!loading" max-width="400" prepend-icon="mdi-shield-key-outline" title="CREATE ZK PROOF">
      <template v-slot:text>
        You will create a proof of solution for the <b>{{ currentLevel }}</b> Sudoku puzzle on <b>{{ dateNow }}</b> <br>
        <br>
        Write your password to continue:
      </template>
      <v-text-field :append-inner-icon="showPass ? 'mdi-eye' : 'mdi-eye-off'" v-model="password"
        :type="showPass ? 'text' : 'password'" label="Password" @click:append-inner="showPass = !showPass"
        class="mx-6"></v-text-field>
      <template v-slot:actions>
        <v-btn class="ms-auto" color="red" text="Cancel" @click="dialog = false"></v-btn>
        <v-spacer></v-spacer>
        <v-btn class="ms-auto" color="green" text="Create" @click="proof()"></v-btn>
      </template>
    </v-card>
    <v-card v-else max-width="400" prepend-icon="mdi-shield-key-outline" title="CREATE ZK PROOF">
      <template v-slot:text>
        You will create a proof of solution for the <b>{{ currentLevel }}</b> Sudoku puzzle on <b>{{ dateNow }}</b> <br>
      </template>
      <div class="text-center">
        <v-progress-circular class="mb-16 mt-5" color="black" indeterminate :size="100"
          :width="10"></v-progress-circular>
      </div>
    </v-card>
  </v-dialog>
  <v-snackbar v-model="notify['active']" location="top right" class="mt-16 cursor-pointer" :color="notify.color"
    @click="notify.active = false">
    <v-icon :icon="notify.icon"></v-icon> <span>{{ notify.text }}</span>
  </v-snackbar>
</template>

<script>
import { groth16 } from 'snarkjs'
import { useAppStore } from '../stores/app'
import api from '@/services/api'
export default {
  setup() {
    const state = useAppStore();
    return { state }
  },
  async mounted() {
    try {
      const responseWasm = await fetch('/circuits/sudokus/circuit.wasm');
      const wasm = await responseWasm.arrayBuffer();
      const responseZkey = await fetch('/circuits/sudokus/circuit_0001.zkey');
      this.zkey = await responseZkey.arrayBuffer();
      this.wc = await this.getWitnessCalculator(wasm);

    } catch (error) {
      console.error('Error loading files WASM & ZKEY: ', error);
    }
    this.getSudoku('easy');
  },
  data: () => ({
    dateNow: new Date().toISOString().slice(0, 10),
    currentLevel: null,
    puzzle: [],
    puzzleOrg: '',
    dialog: false,
    showPass: false,
    password: '',
    notify: {
      active: false,
      text: '',
      color: '',
      icon: ''
    },
    loading: false
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
    handleBeforeInput(event, i, k) {
      const char = event.data;
      const isDigit = /^[1-9]$/.test(char);

      event.preventDefault();

      if (isDigit) {
        this.puzzle[i][k] = Number(char) + 'b';
      }
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
    async getSudoku(level) {
      this.currentLevel = level;
      this.puzzle = [];
      const date = this.dateNow.replaceAll('-', '');
      const response = await api.get(`/sudoku/?date=${date}&level=${level}`)
      if (response.ok) {
        this.puzzle = response.res.data.problem;
        this.puzzleOrg = JSON.stringify(response.res.data.problem);
      }
      else {
        this.notify = {
          active: true,
          text: "There is an error in geting sudoku data.",
          color: "red-lighten-1",
          icon: "mdi-alert-circle"
        }
      }
    },
    async getUser() {
      const response = await api.get(`/user?dni=${this.state.user.dni}`);
      if (response.ok) return response.res.data
      else return false
    },
    async proof() {
      this.loading = true;
      const userData = await this.getUser();
      if (!userData) {
        this.notify = {
          active: true,
          text: "There is an error in geting user data.",
          color: "red-lighten-1",
          icon: "mdi-alert-circle"
        }
        this.loading = false;
        this.dialog = false;
        return
      }
      const solved = this.puzzle.map(row => row.map(i => {
        if (typeof i === 'string') {
          return Number(i.replace('b', ''))
        }
        else {
          return i
        }
      }));
      try {
        const input = {
          pass: this.textToAscii(this.password).join(''),
          dni: this.dniToArray(this.state.user.dni),
          hashBD: userData.hash,
          problem: JSON.parse(this.puzzleOrg),
          solution: solved
        }
        const witnessBin = await this.wc.calculateWTNSBin(input, true);

        const { proof, publicSignals } = await groth16.prove(
          new Uint8Array(this.zkey),
          witnessBin
        );
        const date = this.dateNow.replaceAll('-', '');
        api.post('/solve', {
          public_data: this.toBase64(publicSignals),
          proof_data: this.toBase64(proof),
          date: date,
          level: this.currentLevel
        }).then((res) => {
          if (!res.ok) {throw new Error("Error saving proof");}
          this.notify = {
            active: true,
            text: "The proof creation was successful",
            color: "green-lighten-3",
            icon: "mdi-check-circle-outline"
          }
          this.dialog = false;
        }).catch(err => {
          console.error(err)
          this.notify = {
            active: true,
            text: "There is an error saving the proof in server",
            color: "red-lighten-1",
            icon: "mdi-alert-circle"
          }
          this.dialog = false;
        }).finally(() => {
          this.loading = false;
        })
      } catch {
        this.notify = {
          active: true,
          text: "There is an error in the proof inputs. Please check your password and your sudoku answers.",
          color: "red-lighten-1",
          icon: "mdi-alert-circle"
        }
        this.dialog = false;
        this.loading = false;
      }

    }
  }
}
</script>

<style>
.selected {
  border: solid 2px black !important;
  color: black !important;
}

.no-caret {
  caret-color: transparent;
  cursor: pointer;
}
</style>
