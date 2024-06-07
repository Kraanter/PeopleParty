// automatically generated by the FlatBuffers compiler, do not modify

/* eslint-disable @typescript-eslint/no-unused-vars, @typescript-eslint/no-explicit-any, @typescript-eslint/no-non-null-assertion */

import * as flatbuffers from 'flatbuffers'

export class CrazyCountingPlayerTimePayload {
  bb: flatbuffers.ByteBuffer | null = null
  bb_pos = 0
  __init(i: number, bb: flatbuffers.ByteBuffer): CrazyCountingPlayerTimePayload {
    this.bb_pos = i
    this.bb = bb
    return this
  }

  static getRootAsCrazyCountingPlayerTimePayload(
    bb: flatbuffers.ByteBuffer,
    obj?: CrazyCountingPlayerTimePayload
  ): CrazyCountingPlayerTimePayload {
    return (obj || new CrazyCountingPlayerTimePayload()).__init(
      bb.readInt32(bb.position()) + bb.position(),
      bb
    )
  }

  static getSizePrefixedRootAsCrazyCountingPlayerTimePayload(
    bb: flatbuffers.ByteBuffer,
    obj?: CrazyCountingPlayerTimePayload
  ): CrazyCountingPlayerTimePayload {
    bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH)
    return (obj || new CrazyCountingPlayerTimePayload()).__init(
      bb.readInt32(bb.position()) + bb.position(),
      bb
    )
  }

  timeLeft(): number {
    const offset = this.bb!.__offset(this.bb_pos, 4)
    return offset ? this.bb!.readUint32(this.bb_pos + offset) : 0
  }

  static startCrazyCountingPlayerTimePayload(builder: flatbuffers.Builder) {
    builder.startObject(1)
  }

  static addTimeLeft(builder: flatbuffers.Builder, timeLeft: number) {
    builder.addFieldInt32(0, timeLeft, 0)
  }

  static endCrazyCountingPlayerTimePayload(builder: flatbuffers.Builder): flatbuffers.Offset {
    const offset = builder.endObject()
    return offset
  }

  static createCrazyCountingPlayerTimePayload(
    builder: flatbuffers.Builder,
    timeLeft: number
  ): flatbuffers.Offset {
    CrazyCountingPlayerTimePayload.startCrazyCountingPlayerTimePayload(builder)
    CrazyCountingPlayerTimePayload.addTimeLeft(builder, timeLeft)
    return CrazyCountingPlayerTimePayload.endCrazyCountingPlayerTimePayload(builder)
  }
}
