// automatically generated by the FlatBuffers compiler, do not modify

/* eslint-disable @typescript-eslint/no-unused-vars, @typescript-eslint/no-explicit-any, @typescript-eslint/no-non-null-assertion */

import * as flatbuffers from 'flatbuffers';

export class PausePayloadType {
  bb: flatbuffers.ByteBuffer|null = null;
  bb_pos = 0;
  __init(i:number, bb:flatbuffers.ByteBuffer):PausePayloadType {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

static getRootAsPausePayloadType(bb:flatbuffers.ByteBuffer, obj?:PausePayloadType):PausePayloadType {
  return (obj || new PausePayloadType()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

static getSizePrefixedRootAsPausePayloadType(bb:flatbuffers.ByteBuffer, obj?:PausePayloadType):PausePayloadType {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new PausePayloadType()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

pause():boolean {
  const offset = this.bb!.__offset(this.bb_pos, 4);
  return offset ? !!this.bb!.readInt8(this.bb_pos + offset) : false;
}

static startPausePayloadType(builder:flatbuffers.Builder) {
  builder.startObject(1);
}

static addPause(builder:flatbuffers.Builder, pause:boolean) {
  builder.addFieldInt8(0, +pause, +false);
}

static endPausePayloadType(builder:flatbuffers.Builder):flatbuffers.Offset {
  const offset = builder.endObject();
  return offset;
}

static createPausePayloadType(builder:flatbuffers.Builder, pause:boolean):flatbuffers.Offset {
  PausePayloadType.startPausePayloadType(builder);
  PausePayloadType.addPause(builder, pause);
  return PausePayloadType.endPausePayloadType(builder);
}
}
