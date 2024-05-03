import { defineStore } from 'pinia'

import * as flatbuffers from 'flatbuffers'
import {
  MessageType,
  HostPayloadType,
  JoinPayloadType,
  Message,
} from './../flatbuffers/messageClass' // Import generated TypeScript code

const baseUrl = 'ws://localhost:7899';

export const useWebSocketStore = defineStore('websocket', {
  state: () => ({
    websocket: null as WebSocket | null,
    listeners: [] as Function[]
  }),
  actions: {
    host() {
      this.websocket = new WebSocket(baseUrl + '/host');
      this.websocket.binaryType = 'arraybuffer';
      this.setUpListeners();
    },
    join(roomId: string, name: string) {
      this.websocket = new WebSocket(baseUrl + `/join/${roomId}/${name}`);
      this.websocket.binaryType = 'arraybuffer';
      this.setUpListeners();
    },
    sendMessage(message: string) {
      if (this.websocket) {
        // TODO: flatbuffer stuff, not needed yet because no messages will be sent yet.
        this.websocket.send(message);
      } else {
        console.error('WebSocket is not initialized.');
      }
    },
    setUpListeners() {
      if (this.websocket) {
        this.websocket.onopen = () => {
          console.log('WebSocket connection established.')
        }

        this.websocket.onmessage = (event) => {
          const uintarray = new Uint8Array(event.data)
          this.handleMessage(uintarray)
        }

        this.websocket.onclose = (event) => {
          // set joining to false, maybe need a better error handling for when the connection is closed
          this.listeners.forEach(listener => listener(false));
          console.log('WebSocket connection closed: ', event)
        }
      }
    },
    subscribe(callback: Function) {
      this.listeners.push(callback)
      // Return an unsubscribe function
      return () => {
        const index = this.listeners.indexOf(callback)
        if (index !== -1) {
          this.listeners.splice(index, 1)
        }
      }
    },
    handleMessage(data: Uint8Array) {
      const buf = new flatbuffers.ByteBuffer(data);
      const receivedMessage = Message.getRootAsMessage(buf);

      switch (receivedMessage.type()) {
        case MessageType.Host: {
          const hostPayload = receivedMessage.payload(new HostPayloadType());
          this.listeners.forEach(listener => listener(hostPayload.roomId()));
          break;
        }
        case MessageType.Join: {
          const joinPayload = receivedMessage.payload(new JoinPayloadType());
          this.listeners.forEach(listener => listener(joinPayload.success()));
          break;
        }
        default: {
          console.log('Received Unknown Message Type');
          break;
        }
      }
    }
  }
});
