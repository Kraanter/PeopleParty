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

const baseUrl = `ws${window.location.protocol === 'https:' ? 's' : ''}:${window.location.host}/confetti`

export enum ViewState {
  None,
  PartyPrep,
  MiniGame,
  Leaderboard
}

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
  const partyCode = ref<string | null>('')
  const players = ref<Player[]>([])
  const leaderboard = ref<Leaderboard>({ time_left: 0, players: [] })
  const route = useRoute()
  const isHosting = computed(() => route.name?.toString().toLowerCase() === 'host')
  const playerCount = computed(() => players.value.length)
  const viewState = ref<ViewState>(ViewState.None)

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
        viewState.value = ViewState.PartyPrep
        const hostPayload = receivedMessage.payload(new HostPayloadType())
        partyCode.value = hostPayload.roomId().toString()
        break
      }
      case MessageType.Join: {
        viewState.value = ViewState.PartyPrep
        const joinPayload = receivedMessage.payload(new JoinPayloadType())
        listeners.value.forEach((listener) => listener(joinPayload.success()))
        break
      }
      case MessageType.MiniGame: {
        viewState.value = ViewState.MiniGame
        const miniGamePayload = receivedMessage.payload(new MiniGamePayloadType())
        listeners.value.forEach((listener) => listener(miniGamePayload))
        break
      }
      case MessageType.PartyPrep: {
        viewState.value = ViewState.PartyPrep
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
              names.push({ name: decodeURI(payload.players(i)?.name() ?? '') })
            }
            players.value = names.filter((p) => p && !!p.name)
            break
          }
        }
        break
      }
      case MessageType.Leaderboard: {
        viewState.value = ViewState.Leaderboard
        const leaderboardPayload: LeaderboardPayloadType = receivedMessage.payload(new LeaderboardPayloadType())
        switch (leaderboardPayload.leaderboardtype()) {
          case LeaderboardType.LeaderboardInformation: {
            const payload: LeaderboardInformationPayload = leaderboardPayload.leaderboardpayload(new LeaderboardInformationPayload())
            const newEntries: LeaderboardPlayer[] = []
            for (let i = 0; i < payload.leaderboardLength(); i++) {
              newEntries.push({
                name: decodeURI(payload.leaderboard(i)?.name() ?? ''),
                score: Number(payload.leaderboard(i)?.score()) ?? 0
              })
            }
            leaderboard.value =  {
              time_left: Number(payload.leaderboardTimeLeft()),
              players: newEntries.filter((p) => p && !!p.name)
            }
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
    isHosting,
    players,
    leaderboard,
    playerCount,
    viewState
  }
})
