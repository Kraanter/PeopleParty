// automatically generated by the FlatBuffers compiler, do not modify

/* eslint-disable @typescript-eslint/no-unused-vars, @typescript-eslint/no-explicit-any, @typescript-eslint/no-non-null-assertion */

import * as flatbuffers from 'flatbuffers'

export class CountingClientDataPayload {
  bb: flatbuffers.ByteBuffer | null = null
  bb_pos = 0
  __init(i: number, bb: flatbuffers.ByteBuffer): CountingClientDataPayload {
    this.bb_pos = i
    this.bb = bb
    return this
  }

  static getRootAsCountingClientDataPayload(
    bb: flatbuffers.ByteBuffer,
    obj?: CountingClientDataPayload
  ): CountingClientDataPayload {
    return (obj || new CountingClientDataPayload()).__init(
      bb.readInt32(bb.position()) + bb.position(),
      bb
    )
  }

  static getSizePrefixedRootAsCountingClientDataPayload(
    bb: flatbuffers.ByteBuffer,
    obj?: CountingClientDataPayload
  ): CountingClientDataPayload {
    bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH)
    return (obj || new CountingClientDataPayload()).__init(
      bb.readInt32(bb.position()) + bb.position(),
      bb
    )
  }

  newInt(): number {
    const offset = this.bb!.__offset(this.bb_pos, 4)
    return offset ? this.bb!.readInt16(this.bb_pos + offset) : 0
  }

  static startCountingClientDataPayload(builder: flatbuffers.Builder) {
    builder.startObject(1)
  }

  static addNewInt(builder: flatbuffers.Builder, newInt: number) {
    builder.addFieldInt16(0, newInt, 0)
  }

  static endCountingClientDataPayload(builder: flatbuffers.Builder): flatbuffers.Offset {
    const offset = builder.endObject()
    return offset
  }

  static createCountingClientDataPayload(
    builder: flatbuffers.Builder,
    newInt: number
  ): flatbuffers.Offset {
    CountingClientDataPayload.startCountingClientDataPayload(builder)
    CountingClientDataPayload.addNewInt(builder, newInt)
    return CountingClientDataPayload.endCountingClientDataPayload(builder)
  }
}
