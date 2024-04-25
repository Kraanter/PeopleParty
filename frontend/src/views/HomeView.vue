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
                <button @click="sendMessage(roomID)">Send</button>
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
import { MessageType, HostPayloadType, JoinPayloadType, GameStatePayloadType, GameStateType, Message, Payload, GameStatePayload, CountingClientDataPayload, CountingGameStatePayload } from './../flatbuffers/messageClass'; // Import generated TypeScript code
import * as Buffer from 'buffer';
import { unionToPayload } from "@/flatbuffers/payload";
import { unionToGameStatePayload } from "@/flatbuffers/game-state-payload";

export default defineComponent({
    name: "ViewPage",
    data() {
        return {
            isInRoom: false,
            message: '',
            roomID: 0,
            socket: null as WebSocket | null,
            responseTime: 0 as Number, //in miliseconds
            sendTime: null as Date | null,
        }
    },
    mounted() {

    },
    methods: {
        handleMessage(messageData: Uint8Array) {
            // Create a flatbuffer buffer from the received data
            const buf = new flatbuffers.ByteBuffer(messageData);

            // Get the root message
            const receivedMessage = Message.getRootAsMessage(buf);

            // Switch case based on message type
            switch (receivedMessage.type()) {
                case MessageType.Host: {
                    // Access HostPayload
                    const hostPayload = receivedMessage.payload(new HostPayloadType());
                    // Process host payload...
                    console.log("Received Host Payload: ", hostPayload.roomId());
                    this.roomID = hostPayload.roomId();
                    break;
                }
                case MessageType.Join: {
                    // Access JoinPayload
                    const joinPayload = receivedMessage.payload(new JoinPayloadType());
                    // Process join payload...
                    console.log("Received Join Payload: ", joinPayload.success());
                    break;
                }
                case MessageType.GameState: {
                    // Access GameStatePayload
                    // No payload for GameState
                    console.log("Received Game State Message");
                    break;
                }
                default: {
                    // Handle unknown message type
                    console.log("Received Unknown Message Type");
                    break;
                }
            }
        },
        sendMessage(newInt: number) {
            if (this.socket) {
                // let builder = new flatbuffers.Builder();

                // let hostPayload = HostPayloadType.createHostPayloadType(builder, newInt);

                // let messagePacket = Message.createMessage(builder, MessageType.Host, Payload.HostPayloadType, hostPayload);

                // builder.finish(messagePacket);

                // var dec = new TextDecoder();
                // let data = dec.decode(builder.asUint8Array());

                // this.socket.send(data);


                let builder = new flatbuffers.Builder();

                // for some reason uncommenting the next lines work

                //let CountingClientPayloadData = CountingClientDataPayload.createCountingClientDataPayload(builder, 30);

                //let GameStatePayloadData = GameStatePayloadType.createGameStatePayloadType(builder, GameStateType.CountingClientData, GameStatePayload.CountingClientDataPayload, CountingClientPayloadData);

                //let messagePacket = Message.createMessage(builder, MessageType.GameState, Payload.GameStatePayloadType, GameStatePayloadData);

                let ff2 = CountingClientDataPayload.createCountingClientDataPayload(builder, 30);

                let ff = GameStatePayloadType.createGameStatePayloadType(builder, GameStateType.CountingClientData, GameStatePayload.CountingClientDataPayload, ff2);

                let messagePacket2 = Message.createMessage(builder, MessageType.GameState, Payload.GameStatePayloadType, ff);

                builder.finish(messagePacket2);

                var dec = new TextDecoder();
                let data = dec.decode(builder.asUint8Array());

                this.socket.send(data);



                // let unpackmessage = Message.getRootAsMessage(new flatbuffers.ByteBuffer(builder.asUint8Array()));
                // console.log('Unpacked message: ', unpackmessage.type());
                // switch (unpackmessage.type()) {
                //     case MessageType.Host: {
                //         break;
                //     }
                //     case MessageType.Join: {
                //         break;
                //     }
                //     case MessageType.GameState: {
                //         // Access GameStatePayload
                //         // No payload for GameState
                //         const gamestatePayload = unpackmessage.payload(new GameStatePayloadType());
                //         const clientsendData =  gamestatePayload.gamestatepayload(new CountingClientDataPayload());
                //         console.log('Received data: ', clientsendData.newInt());
                //         break;
                //     }
                //     default: {
                //         // Handle unknown message type
                //         console.log("Received Unknown Message Type");
                //         break;
                //     }
                // }

                // GameStatePayloadType.startGameStatePayloadType(builder);
                // GameStatePayloadType.addGamestatetype(builder, GameStateType.CountingClientData);
                // GameStatePayloadType.addGamestatepayload(builder, CountingClientPayload);
                // let GameStatePayload = GameStatePayloadType.endGameStatePayloadType(builder);

                // Message.startMessage(builder);
                // Message.addType(builder, MessageType.GameState);
                // Message.addPayload(builder, GameStatePayload);
                // let messagePacket = Message.endMessage(builder);

                // builder.finish(messagePacket);

                // // send the message

                // this.socket.send(builder.asUint8Array());

                // const buffer = Buffer.Buffer.from(builder.asUint8Array());
                // const dataAsString = buffer.toString('utf-8');
                // console.log('Sending message: ', dataAsString);
                // this.socket.send(dataAsString);
            }
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
                    var enc = new TextEncoder();
                    let data = enc.encode(event.data);
                    console.log('Received data of size: ', data.byteLength);
                    this.handleMessage(data);
                    // let buf = new flatbuffers.ByteBuffer(data);

                    // let gameState = Message.getRootAsMessage(buf);
                    // console.log('Message received: ', gameState.type(), gameState.payload());

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
