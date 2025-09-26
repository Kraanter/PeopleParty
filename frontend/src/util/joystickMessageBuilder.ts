import { GameStatePayload } from '@/flatbuffers/game-state-payload'
import { GameStateType } from '@/flatbuffers/game-state-type'
import { JoystickDataPayload } from '@/flatbuffers/joystick-data-payload'
import { MessageType } from '@/flatbuffers/message-type'
import { MiniGamePayloadType } from '@/flatbuffers/mini-game-payload-type'
import { Payload } from '@/flatbuffers/payload'
import * as flatbuffers from 'flatbuffers'
import { buildMessage } from './flatbufferMessageBuilder'
import { JoystickEventType } from '@/flatbuffers/joystick-event-type'
import { JoystickEventPayload } from '@/flatbuffers/joystick-event-payload'
import { useWebSocketStore } from '@/stores/confettiStore'

const websocketStore = useWebSocketStore()


export const sendPlayerAction = (minigameName: string, x: number, y: number) => {
  const builder = new flatbuffers.Builder()

  const playerInput = JoystickDataPayload.createJoystickDataPayload(
    builder,
    x,
    y,
  )

  const miniGame = builder.createString(minigameName)

  const miniGamePayload = MiniGamePayloadType.createMiniGamePayloadType(
    builder,
    miniGame,
    GameStateType.JoystickData,
    GameStatePayload.JoystickDataPayload,
    playerInput
  )

  websocketStore.sendMessage(
    buildMessage(builder, miniGamePayload, MessageType.MiniGame, Payload.MiniGamePayloadType)
  )
}

export const sendPlayerEvent = (minigameName: string, event: string) => {
  const builder = new flatbuffers.Builder()

  let eventType: JoystickEventType;
  switch (event) {
    case 'start':
      eventType = JoystickEventType.Start
      break
    case 'stop':
      eventType = JoystickEventType.Stop
      break
    default:
      eventType = JoystickEventType.Stop
      break
  }

  const playerInput = JoystickEventPayload.createJoystickEventPayload(
    builder,
    eventType,
  )

  const miniGame = builder.createString(minigameName)

  const miniGamePayload = MiniGamePayloadType.createMiniGamePayloadType(
    builder,
    miniGame,
    GameStateType.JoystickEvent,
    GameStatePayload.JoystickEventPayload,
    playerInput
  )

  websocketStore.sendMessage(
    buildMessage(builder, miniGamePayload, MessageType.MiniGame, Payload.MiniGamePayloadType)
  )
}