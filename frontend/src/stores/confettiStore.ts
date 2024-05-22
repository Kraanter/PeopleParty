import { defineStore } from 'pinia'

import * as flatbuffers from 'flatbuffers'
import {
  HostPayloadType,
  JoinPayloadType,
  Message,
  MessageType,
  MiniGamePayloadType,
  PartyPrepHostInformationPayload,
  PartyPrepPayload,
  PartyPrepPayloadType,
  PartyPrepType
} from './../flatbuffers/messageClass'
import { computed, ref } from 'vue'

const baseUrl = `ws${window.location.protocol === 'https:' ? 's' : ''}:${window.location.host}/confetti`

export const useWebSocketStore = defineStore('websocket', () => {
  const websocket = ref<WebSocket | null>(null)
  const listeners = ref<Function[]>([])
  const partyCode = ref<string | null>('')
  const players = ref<{ name: string }[]>([])
  const isHost = computed(() => !!partyCode.value)
  const playerCount = computed(() => players.value.length)

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
        partyCode.value = hostPayload.roomId().toString()
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
      case MessageType.PartyPrep: {
        console.log('party prep message')
        const partyPrepPayload: PartyPrepPayloadType = receivedMessage.payload(
          new PartyPrepPayloadType()
        )
        switch (partyPrepPayload.partypreptype()) {
          case PartyPrepType.PartyPrepHostInformation: {
            const payload: PartyPrepHostInformationPayload = partyPrepPayload.partypreppayload(
              new PartyPrepHostInformationPayload()
            )
            const names = []
            for (let i = 0; i < payload.playersLength(); i++) {
              names.push(payload.players(i)?.name())
            }
            players.value = names
            break
          }
        }
        break
      }
      default: {
        console.log('Received Unknown Message Type')
        break
      }
    }
  }

  return { host, join, sendMessage, subscribe, partyCode, isHost, players, playerCount }
})
