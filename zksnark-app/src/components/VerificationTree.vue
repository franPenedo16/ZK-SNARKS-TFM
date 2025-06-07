<template>
    <AppNavBar />
    <v-container class="fill-height" max-width="900">
        <v-timeline v-if="items.length > 0" density="default">
            <v-timeline-item v-for="item in items" :key="item.id"
                :dot-color="item.verifiers_count > 0 ? '#81C784' : '#546E7A'" size="small" class="">
                <v-alert v-if="item.verifiers_count > 0" color="#81C784" icon="mdi-check-bold" :value="true">
                    The user <b>{{ item.username.toUpperCase() }}</b> has solved the <b>{{ item.level }}</b>
                    sudoku of the <b>{{
                        item.date }}</b>.
                    <br>
                    <br>
                    <div class="d-flex justify-space-between">
                        <v-tooltip location="bottom center" origin="bottom end" no-click-animation>
                            <template v-slot:activator="{ props }">
                                <span v-bind="props">Verified by <b>{{ item.verifiers_count }}</b> users.</span>
                            </template>
                            <span v-if="item.verifiers_count > 1">{{ item.verifiers_username[0] }} and others
                                ....</span>
                            <span v-else>{{ item.verifiers_username[0] }}</span>
                        </v-tooltip>

                        <v-btn text="Verify" color="gray" class="border" @click="verify(item.id)"></v-btn>
                    </div>
                </v-alert>
                <v-alert v-else color="#546E7A" icon="mdi-human-queue" :value="true">
                    The user <b>{{ item.username.toUpperCase() }}</b> has solved the <b>{{ item.level }}</b>
                    sudoku of the <b>{{
                        item.date }}</b>.
                    <br>
                    <br>
                    <div class="d-flex justify-space-between">
                        <span class="mr-6">Has not been verified by anyone</span>
                        <v-btn text="Verify" color="gray" class="border" @click="verify(item.id)"></v-btn>
                    </div>
                </v-alert>
            </v-timeline-item>
        </v-timeline>
        <v-progress-circular v-else class="ml-16 w-100" color="black" indeterminate :size="100"
            :width="5"></v-progress-circular>
    </v-container>
    <v-snackbar v-model="notify['active']" location="top right" class="mt-10 cursor-pointer" :color="notify.color"
        @click="notify.active = false">
        <v-icon :icon="notify.icon"></v-icon> <span>{{ notify.text }}</span>
    </v-snackbar>
    <v-snackbar v-model="notify_2['active']" location="top right" class="mt-16 pt-16 cursor-pointer"
        :color="notify_2.color" @click="notify_2.active = false">
        <v-icon :icon="notify_2.icon"></v-icon> <span>{{ notify_2.text }}</span>
    </v-snackbar>
    <v-fab location="right bottom" color="red" :app="true" icon="mdi-account-cancel" @click="getBlocked()"></v-fab>
    <v-bottom-sheet v-model="blockUserList" inset>
        <v-list>
            <v-list-subheader color="red" title="Blocked users"></v-list-subheader>
            <v-list-item> <v-text-field v-model="userToBlock" append-inner-icon="mdi-send-circle-outline"
                    label="Add a user to block" @click:append-inner="addBlocked()"></v-text-field></v-list-item>
            <v-list-item v-for="user in users" :title="user.username">
                <template #prepend>
                    <v-icon color="red" @click="removeBlocked(user.id)">mdi-close-circle-outline</v-icon>
                </template>
            </v-list-item>
        </v-list>
    </v-bottom-sheet>
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
        const ResponseVk = await fetch('/circuits/sudokus/verification_key.json');
        this.vk = await ResponseVk.json()
        this.getResolved();
    },
    data: () => ({
        items: [],
        notify: {
            active: false,
            text: '',
            color: '',
            icon: ''
        },
        notify_2: {
            active: false,
            text: '',
            color: '',
            icon: ''
        },
        users: [],
        userToBlock: '',
        blockUserList: false
    }),

    methods: {
        async getResolved() {
            const response = await api.get(`/resolved`);
            if (response.ok) {
                this.items = response.res.data;
            }
            else {
                this.notify = {
                    active: true,
                    text: "There is an error in geting resolved data from the users",
                    color: "red-lighten-1",
                    icon: "mdi-alert-circle"
                }
            }
        },
        async getBlocked() {
            this.blockUserList = true;
            const response = await api.get(`/blocked`);
            if (response.ok) {
                this.users = response.res.data;
            }
            else {
                this.notify = {
                    active: true,
                    text: "There is an error geting blocked users",
                    color: "red-lighten-1",
                    icon: "mdi-alert-circle"
                }
            }
        },
        async addBlocked() {
            api.post('/blocked', {
                username: this.userToBlock
            }).then((res) => {
                if (!res.ok) { throw new Error("Error saving blocked user"); }
                this.notify_2 = {
                    active: true,
                    text: "The user was blocked successfuly",
                    color: "green-lighten-3",
                    icon: "mdi-check-circle-outline"
                }
            }).catch(err => {
                console.error(err)
                this.notify_2 = {
                    active: true,
                    text: "There is an error blocking this user in server",
                    color: "red-lighten-1",
                    icon: "mdi-alert-circle"
                }
            }).finally(() => {
                this.userToBlock = '';
                this.getBlocked();
                this.getResolved();
            })
        },
        async removeBlocked(id) {
            api.delete('/blocked', {
                data: {
                    id: id
                }
            }).then((res) => {
                if (!res.ok) { throw new Error("Error unblocking user"); }
                this.notify_2 = {
                    active: true,
                    text: "The user was successfully unblocked",
                    color: "green-lighten-3",
                    icon: "mdi-check-circle-outline"
                }
            }).catch(err => {
                console.error(err)
                this.notify_2 = {
                    active: true,
                    text: "There is an error unblocking this user in server",
                    color: "red-lighten-1",
                    icon: "mdi-alert-circle"
                }
            }).finally(() => {
                this.getBlocked();
                this.getResolved();
            })
        },
        async verify(id) {
            const response = await api.get(`/solution?id=${id}`);
            if (!response.ok) {
                this.notify = {
                    active: true,
                    text: "There is an error in geting data for verified",
                    color: "red-lighten-1",
                    icon: "mdi-alert-circle"
                }
                return
            }
            const verifyObj = response.res.data;

            let problem_publicInputs = [];
            problem_publicInputs.push([]);
            let countArray = 0;
            let count = 0;
            let equals = true;
            verifyObj.public_inputs.map((input, index) => {

                if (index > 1) {
                    if (verifyObj.problem[countArray][count] != input) equals = false;
                    count += 1;
                }

                if (count === 9) {
                    countArray += 1;
                    count = 0;
                }
            })

            if (!equals) {
                this.notify = {
                    active: true,
                    text: "The sudoku don't match the public input of the proof",
                    color: "red-lighten-1",
                    icon: "mdi-alert-circle"
                }
                return
            }

            if (verifyObj.public_inputs[0] != verifyObj.user_hash) {
                this.notify = {
                    active: true,
                    text: "The password hash of the user don't match the public input of the proof",
                    color: "red-lighten-1",
                    icon: "mdi-alert-circle"
                }
                return
            }

            const isValid = await groth16.verify(this.vk, verifyObj.public_inputs, verifyObj.proof);

            const owner = this.state.user.id === verifyObj.user_id;

            if (isValid && !owner) {
                this.notify = {
                    active: true,
                    text: "Solution verified. Publishing result.... ",
                    color: "green-lighten-3",
                    icon: "mdi-check-circle-outline"
                }
            }
            else if (isValid && owner) {
                this.notify = {
                    active: true,
                    text: "Solution verified, but self-verification is not allowed.",
                    color: "orange-lighten-3",
                    icon: "mdi-progress-alert"
                }
                return
            } else {
                this.notify = {
                    active: true,
                    text: "This is a false proof!",
                    color: "red-lighten-1",
                    icon: "mdi-alert-circle"
                }
                return
            }

            api.post('/verify', {
                solved: verifyObj.resolved_id
            }).then((res) => {
                if (!res.ok) { throw new Error("Error saving verificatios"); }
                this.notify_2 = {
                    active: true,
                    text: "The verify was saved successful",
                    color: "green-lighten-3",
                    icon: "mdi-check-circle-outline"
                }
                this.getResolved();
            }).catch(err => {
                console.error(err)
                this.notify_2 = {
                    active: true,
                    text: "There is an error saving the verification in server",
                    color: "red-lighten-1",
                    icon: "mdi-alert-circle"
                }
            })
        }
    }
}
</script>
