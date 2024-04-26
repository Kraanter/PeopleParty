// automatically generated by the FlatBuffers compiler, do not modify

/* eslint-disable @typescript-eslint/no-unused-vars, @typescript-eslint/no-explicit-any, @typescript-eslint/no-non-null-assertion */

import * as flatbuffers from 'flatbuffers'

export class HostPayloadType {
  bb: flatbuffers.ByteBuffer | null = null
  bb_pos = 0
  __init(i: number, bb: flatbuffers.ByteBuffer): HostPayloadType {
    this.bb_pos = i
    this.bb = bb
    return this
  }

  static getRootAsHostPayloadType(
    bb: flatbuffers.ByteBuffer,
    obj?: HostPayloadType
  ): HostPayloadType {
    return (obj || new HostPayloadType()).__init(bb.readInt32(bb.position()) + bb.position(), bb)
  }

  static getSizePrefixedRootAsHostPayloadType(
    bb: flatbuffers.ByteBuffer,
    obj?: HostPayloadType
  ): HostPayloadType {
    bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH)
    return (obj || new HostPayloadType()).__init(bb.readInt32(bb.position()) + bb.position(), bb)
  }

  roomId(): number {
    const offset = this.bb!.__offset(this.bb_pos, 4)
    return offset ? this.bb!.readInt16(this.bb_pos + offset) : 0
  }

  static startHostPayloadType(builder: flatbuffers.Builder) {
    builder.startObject(1)
  }

  static addRoomId(builder: flatbuffers.Builder, roomId: number) {
    builder.addFieldInt16(0, roomId, 0)
  }

  static endHostPayloadType(builder: flatbuffers.Builder): flatbuffers.Offset {
    const offset = builder.endObject()
    return offset
  }

  static createHostPayloadType(builder: flatbuffers.Builder, roomId: number): flatbuffers.Offset {
    HostPayloadType.startHostPayloadType(builder)
    HostPayloadType.addRoomId(builder, roomId)
    return HostPayloadType.endHostPayloadType(builder)
  }
}
