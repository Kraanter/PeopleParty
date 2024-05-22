import { Message, MessageType, Payload } from '@/flatbuffers/messageClass'
import * as flatbuffers from 'flatbuffers'

export const buildMessage = (builder: flatbuffers.Builder, miniGamePayload: number, messageType: MessageType, payloadType: Payload) => {
  const message = Message.createMessage(
    builder,
    messageType,
    payloadType,
    miniGamePayload
  )
  builder.finish(message)
  return builder.asUint8Array()
}
