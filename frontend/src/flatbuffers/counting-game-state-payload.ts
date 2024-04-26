// automatically generated by the FlatBuffers compiler, do not modify

/* eslint-disable @typescript-eslint/no-unused-vars, @typescript-eslint/no-explicit-any, @typescript-eslint/no-non-null-assertion */

import * as flatbuffers from 'flatbuffers'

import { Object_ } from './object.js'

export class CountingGameStatePayload {
  bb: flatbuffers.ByteBuffer | null = null
  bb_pos = 0
  __init(i: number, bb: flatbuffers.ByteBuffer): CountingGameStatePayload {
    this.bb_pos = i
    this.bb = bb
    return this
  }

  static getRootAsCountingGameStatePayload(
    bb: flatbuffers.ByteBuffer,
    obj?: CountingGameStatePayload
  ): CountingGameStatePayload {
    return (obj || new CountingGameStatePayload()).__init(
      bb.readInt32(bb.position()) + bb.position(),
      bb
    )
  }

  static getSizePrefixedRootAsCountingGameStatePayload(
    bb: flatbuffers.ByteBuffer,
    obj?: CountingGameStatePayload
  ): CountingGameStatePayload {
    bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH)
    return (obj || new CountingGameStatePayload()).__init(
      bb.readInt32(bb.position()) + bb.position(),
      bb
    )
  }

  objects(index: number, obj?: Object_): Object_ | null {
    const offset = this.bb!.__offset(this.bb_pos, 4)
    return offset
      ? (obj || new Object_()).__init(
          this.bb!.__indirect(this.bb!.__vector(this.bb_pos + offset) + index * 4),
          this.bb!
        )
      : null
  }

  objectsLength(): number {
    const offset = this.bb!.__offset(this.bb_pos, 4)
    return offset ? this.bb!.__vector_len(this.bb_pos + offset) : 0
  }

  static startCountingGameStatePayload(builder: flatbuffers.Builder) {
    builder.startObject(1)
  }

  static addObjects(builder: flatbuffers.Builder, objectsOffset: flatbuffers.Offset) {
    builder.addFieldOffset(0, objectsOffset, 0)
  }

  static createObjectsVector(
    builder: flatbuffers.Builder,
    data: flatbuffers.Offset[]
  ): flatbuffers.Offset {
    builder.startVector(4, data.length, 4)
    for (let i = data.length - 1; i >= 0; i--) {
      builder.addOffset(data[i]!)
    }
    return builder.endVector()
  }

  static startObjectsVector(builder: flatbuffers.Builder, numElems: number) {
    builder.startVector(4, numElems, 4)
  }

  static endCountingGameStatePayload(builder: flatbuffers.Builder): flatbuffers.Offset {
    const offset = builder.endObject()
    return offset
  }

  static createCountingGameStatePayload(
    builder: flatbuffers.Builder,
    objectsOffset: flatbuffers.Offset
  ): flatbuffers.Offset {
    CountingGameStatePayload.startCountingGameStatePayload(builder)
    CountingGameStatePayload.addObjects(builder, objectsOffset)
    return CountingGameStatePayload.endCountingGameStatePayload(builder)
  }
}
