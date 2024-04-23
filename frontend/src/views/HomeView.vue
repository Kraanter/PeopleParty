<template>
    <main class="flex justify-center items-center w-full min-h-screen">
        <div class="flex flex-col gap-5 items-center">
            <h1 class="text-title">People Party</h1>

            <div>
                roomid: {{ roomID }} <br>
                response time: {{ responseTime }} ms<br>
            </div>

            <div v-if="isInRoom">
                <label for="message">Message:</label>
                <input class="input-field" type="text" id="message" name="message" v-model="message"><br>
                <button @click="sendMessage(message)">Send</button>
            </div>
            <div v-if="!isInRoom">
                <button @click="hostRoom">Host Room</button><br><br>
                <label for="roomID">Room ID:</label>
                <input class="input-field" type="text" id="roomID" name="roomID" v-model="roomID"><br>
                <button @click="joinRoom">Join Room</button>
            </div>
        </div>
    </main>
</template>


<script lang="ts">
import { defineComponent } from "vue";

import * as flatbuffers from 'flatbuffers';
import { Message } from './../../src/models/message'; // Import generated TypeScript code

export default defineComponent({
    name: "ViewPage",
    data() {
        return {
            isInRoom: false,
            message: '',
            roomID: '',
            socket: null as WebSocket | null,
            responseTime: 0 as Number, //in miliseconds
            sendTime: null as Date | null,
        }
    },
    mounted() {

    },
    methods: {
        sendMessage(message: string) {
            this.sendTime = new Date();
            if (message === '') {
                message = 'ping';
            }
            this.socket?.send(message);
            this.message = '';
        },
        hostRoom() {
            // connect to the websocket and set the roomID that you got from the socket
            this.socket = new WebSocket('ws://localhost:7899/host');
            this.socket.binaryType = "arraybuffer";
            this.setUpListeners();
            this.isInRoom = true;
        },
        joinRoom() {
            this.socket = new WebSocket(`ws://localhost:7899/join/${this.roomID}`);
            this.socket.binaryType = "arraybuffer";
            this.setUpListeners();
            this.isInRoom = true;
        },
        setUpListeners() {
            if (this.socket) {
                this.socket.onopen = () => {
                    console.log('WebSocket connection established.');
                };

                this.socket.onmessage = (event) => {
                    console.log(event.data);

                    var enc = new TextEncoder();
                    let data = enc.encode(event.data);

                    console.log(data);
                    let buf = new flatbuffers.ByteBuffer(data);

                    let gameState = Message.getRootAsMessage(buf);
                    console.log('Message received: ', gameState.type(), gameState.payload());

                    // if (event.data.includes('roomID: ')) {
                    //     this.roomID = event.data.split('roomID: ')[1];
                    // }

                    if (this.sendTime) {
                        this.responseTime = new Date().getTime() - this.sendTime.getTime();
                        this.sendTime = null;
                    }
                };

                this.socket.onclose = (event) => {
                    console.log('WebSocket connection closed: ', event);
                    this.isInRoom = false;
                };
            }
        }
    }
});
</script>


<style scoped>
.input-field {
    color: black;
}
</style>
