import { defineStore } from 'pinia'

const baseUrl = 'ws://localhost:7899';

export const useWebSocketStore = defineStore('websocket', {
  state: () => ({
    websocket: null as WebSocket | null
  }),
  actions: {
    host() {
      this.websocket = new WebSocket(baseUrl + '/host');
      this.websocket.binaryType = 'arraybuffer';
      this.setUpListeners();
    },
    join(roomId: number, name: string) {
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
          console.log('WebSocket connection closed: ', event)
        }
      }
    },
    handleMessage(data: ArrayBuffer) {
      // const buf = new flatbuffers.ByteBuffer(data);
      // const receivedMessage = Message.getRootAsMessage(buf);

      // switch (receivedMessage.type()) {
      //   case MessageType.Host: {
      //     const hostPayload = receivedMessage.payload(new HostPayloadType());
      //     console.log('Received Host Payload: ', hostPayload.roomId());
      //     this.roomID = hostPayload.roomId();
      //     break;
      //   }
      //   case MessageType.Join: {
      //     const joinPayload = receivedMessage.payload(new JoinPayloadType());
      //     console.log('Received Join Payload: ', joinPayload.success());
      //     break;
      //   }
      //   default: {
      //     console.log('Received Unknown Message Type');
      //     break;
      //   }
      // }
    }
  }
});
