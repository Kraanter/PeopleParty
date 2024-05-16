import { Message, MessageType, Payload } from '@/flatbuffers/messageClass';
import * as flatbuffers from 'flatbuffers';

export const buildMessage = (builder: flatbuffers.Builder, miniGamePayload: number) => {

    const message = Message.createMessage(builder, MessageType.MiniGame, Payload.MiniGamePayloadType, miniGamePayload);
    builder.finish(message);
    return builder.asUint8Array();
}