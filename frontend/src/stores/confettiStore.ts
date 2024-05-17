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
        console.log('Received MiniGame Message')
        const miniGamePayload = receivedMessage.payload(new MiniGamePayloadType())
        console.log(listeners.value.length);
        if (miniGamePayload instanceof MiniGamePayloadType) {
          console.log('miniGamePayload is MiniGamePayloadType')
        }
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
