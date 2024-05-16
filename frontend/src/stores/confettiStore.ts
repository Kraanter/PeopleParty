import { defineStore } from 'pinia'

import * as flatbuffers from 'flatbuffers'
import {
  MessageType,
  HostPayloadType,
  JoinPayloadType,
  Message,
  MiniGamePayloadType
} from './../flatbuffers/messageClass'
import { ref } from 'vue'

const baseUrl = `ws${window.location.protocol === 'https:' ? 's' : ''}:${window.location.host}/confetti`

export const useWebSocketStore = defineStore('websocket', () => {
  const websocket = ref<WebSocket | null>(null)
  const listeners = ref<Function[]>([])

  function host() {
    websocket.value = new WebSocket(baseUrl + '/host')
    websocket.value.binaryType = 'arraybuffer'
    setUpListeners()
  }

  function join(roomId: string, name: string) {
    websocket.value = new WebSocket(baseUrl + `/join/${roomId}/${name}`)
    websocket.value.binaryType = 'arraybuffer'
    setUpListeners()
  }

  function sendMessage(message: Uint8Array) {
    if (websocket.value) {
      // TODO: flatbuffer stuff, not needed yet because no messages will be sent yet.
      websocket.value.send(message)
    } else {
      console.error('WebSocket is not initialized.')
    }
  }

  function setUpListeners() {
    if (websocket.value) {
      websocket.value.onopen = () => {
        console.log('WebSocket connection established.')
      }

      websocket.value.onmessage = (event) => {
        const uintarray = new Uint8Array(event.data)
        console.log(uintarray)
        const uintarray2 = Uint8Array.from([16, 0, 0, 0, 0, 0, 10, 0, 14, 0, 6, 0, 7, 0, 8, 0, 10, 0, 0, 0, 0, 0, 2, 3, 16, 0, 0, 0, 0, 0, 10, 0, 14, 0, 0, 0, 7, 0, 8, 0, 10, 0, 0, 0, 0, 0, 0, 1, 12, 0, 0, 0, 0, 0, 6, 0, 8, 0, 4, 0, 6, 0, 0, 0, 4, 0, 0, 0, 10, 0, 0, 0, 156, 0, 0, 0, 132, 0, 0, 0, 116, 0, 0, 0, 100, 0, 0, 0, 84, 0, 0, 0, 68, 0, 0, 0, 52, 0, 0, 0, 36, 0, 0, 0, 20, 0, 0, 0, 4, 0, 0, 0, 148, 255, 255, 255, 192, 17, 96, 62, 68, 193, 33, 61, 160, 255, 255, 255, 2, 225, 128, 61, 239, 99, 119, 63, 172, 255, 255, 255, 172, 29, 86, 63, 111, 165, 183, 62, 184, 255, 255, 255, 84, 49, 42, 62, 253, 153, 254, 62, 196, 255, 255, 255, 110, 13, 55, 63, 76, 239, 37, 63, 208, 255, 255, 255, 189, 73, 222, 62, 199, 81, 99, 62, 220, 255, 255, 255, 214, 51, 107, 63, 6, 33, 3, 61, 232, 255, 255, 255, 178, 57, 89, 62, 115, 77, 185, 62, 244, 255, 255, 255, 221, 75, 110, 63, 245, 71, 122, 63, 8, 0, 12, 0, 4, 0, 8, 0, 8, 0, 0, 0, 15, 121, 7, 63, 129, 153, 192, 62])
        
        let bool = true;
        let nonEqual = []
        for (let i = 0; i < uintarray.length; i++) {
          if (uintarray[i] !== uintarray2[i]) {
            bool = false;
            nonEqual.push(`non equal at index ${i} for ${uintarray[i]} and ${uintarray2[i]}`)
          }
        }
        console.log(bool, nonEqual)
        handleMessage(uintarray)
      }

      websocket.value.onclose = (event) => {
        // set joining to false, maybe need a better error handling for when the connection is closed
        listeners.value.forEach((listener) => listener(false))
        console.log('WebSocket connection closed: ', event)
      }
    }
  }

  function subscribe(callback: Function) {
    listeners.value.push(callback)
    // Return an unsubscribe function
    return () => {
      const index = listeners.value.indexOf(callback)
      if (index !== -1) {
        listeners.value.splice(index, 1)
      }
    }
  }

  function handleMessage(data: Uint8Array) {
    const buf = new flatbuffers.ByteBuffer(data)
    const receivedMessage = Message.getRootAsMessage(buf)

    switch (receivedMessage.type()) {
      case MessageType.Host: {
        const hostPayload = receivedMessage.payload(new HostPayloadType())
        listeners.value.forEach((listener) => listener(hostPayload.roomId()))
        break
      }
      case MessageType.Join: {
        const joinPayload = receivedMessage.payload(new JoinPayloadType())
        listeners.value.forEach((listener) => listener(joinPayload.success()))
        break
      }
      case MessageType.MiniGame: {
        const miniGamePayload = receivedMessage.payload(new MiniGamePayloadType())
        listeners.value.forEach((listener) => listener(miniGamePayload))
        break
      }
      default: {
        console.log('Received Unknown Message Type')
        break
      }
    }
  }

  return { host, join, sendMessage, subscribe }
})
