import { defineStore } from 'pinia'
import * as flatbuffers from 'flatbuffers'
import {
  HostPayloadType,
  JoinPayloadType,
  LeaderboardInformationPayload,
  LeaderboardPayloadType,
  LeaderboardType,
  Message,
  MessageType,
  MiniGamePayloadType,
  PartyPrepHostInformationPayload,
  PartyPrepPayloadType,
  PartyPrepType
} from './../flatbuffers/messageClass'
import { computed, ref } from 'vue'
import { useRoute } from 'vue-router'
import { ViewState, useViewStore } from './viewStore'

const baseUrl = `ws${window.location.protocol === 'https:' ? 's' : ''}:${window.location.host}/confetti`

export type Player = {
  name: string
}

export type Leaderboard = {
  time_left: number
  players: LeaderboardPlayer[]
}

export type LeaderboardPlayer = {
  name: string
  score: number
}

export const useWebSocketStore = defineStore('websocket', () => {
  const websocket = ref<WebSocket | null>(null)
  const listeners = ref<Function[]>([])
  const partyCode = ref<string>('')
  const route = useRoute()
  const isHosting = computed(() => route.name?.toString().toLowerCase() === 'host')
  const clientName = ref<string>('')
  const viewStore = useViewStore()
  const isConnecting = ref(false)

  function host() {
    if (isConnecting.value) return
    isConnecting.value = true
    websocket.value = new WebSocket(baseUrl + '/host')
    websocket.value.binaryType = 'arraybuffer'
    setUpListeners()
  }

  function join(roomId: string, name: string) {
    websocket.value = new WebSocket(baseUrl + `/join/${roomId}/${name}`)
    websocket.value.binaryType = 'arraybuffer'
    clientName.value = name
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
        isConnecting.value = false
      }

      websocket.value.onmessage = (event) => {
        if (event.data == '') return
        const uintarray = new Uint8Array(event.data)
        handleMessage(uintarray)
      }

      websocket.value.onclose = (event) => {
        isConnecting.value = false
        // set joining to false, maybe need a better error handling for when the connection is closed
        listeners.value.forEach((listener) => listener(false))
        console.log('WebSocket connection closed: ', event)
        if (partyCode.value) location.reload()
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
        const hostPayload: HostPayloadType = receivedMessage.payload(new HostPayloadType())
        partyCode.value = hostPayload.roomId().toString()
        clientName.value = 'Host'
        if (isHosting.value) {
          viewStore.setViewState(ViewState.PartyPrep, [])
        }
        break
      }
      case MessageType.Join: {
        viewStore.setViewState(ViewState.PartyPrep)
        const joinPayload = receivedMessage.payload(new JoinPayloadType())
        listeners.value.forEach((listener) => listener(joinPayload.success()))
        break
      }
      case MessageType.MiniGame: {
        viewStore.setViewState(ViewState.MiniGame)
        const miniGamePayload: MiniGamePayloadType = receivedMessage.payload(
          new MiniGamePayloadType()
        )
        viewStore.setViewData(miniGamePayload)
        listeners.value.forEach((listener) => listener(miniGamePayload))
        break
      }
      case MessageType.PartyPrep: {
        const partyPrepPayload: PartyPrepPayloadType = receivedMessage.payload(
          new PartyPrepPayloadType()
        )

        if (partyPrepPayload.partypreppayloadType()) viewStore.setViewState(ViewState.PartyPrep)

        switch (partyPrepPayload.partypreptype()) {
          case PartyPrepType.PartyPrepHostInformation: {
            const payload: PartyPrepHostInformationPayload = partyPrepPayload.partypreppayload(
              new PartyPrepHostInformationPayload()
            )
            const names = []
            for (let i = 0; i < payload.playersLength(); i++) {
              names.push({ name: decodeURI(payload.players(i)?.name() ?? '') })
            }

            viewStore.setViewData(names.filter((p) => p && !!p.name))
            break
          }
        }
        break
      }
      case MessageType.Leaderboard: {
        const leaderboardPayload: LeaderboardPayloadType = receivedMessage.payload(
          new LeaderboardPayloadType()
        )
        if (leaderboardPayload.leaderboardpayloadType())
          viewStore.setViewState(ViewState.Leaderboard, [])

        switch (leaderboardPayload.leaderboardtype()) {
          case LeaderboardType.LeaderboardInformation: {
            const payload: LeaderboardInformationPayload = leaderboardPayload.leaderboardpayload(
              new LeaderboardInformationPayload()
            )
            const newEntries: LeaderboardPlayer[] = []
            for (let i = 0; i < payload.leaderboardLength(); i++) {
              newEntries.push({
                name: decodeURI(payload.leaderboard(i)?.name() ?? ''),
                score: Number(payload.leaderboard(i)?.score()) ?? 0
              })
            }
            viewStore.setViewData({
              time_left: Number(payload.leaderboardTimeLeft()),
              players: newEntries.filter((p) => p && !!p.name)
            })
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

  return {
    host,
    join,
    sendMessage,
    subscribe,
    partyCode,
    clientName,
    isHosting
  }
})
