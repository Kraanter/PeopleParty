// automatically generated by the FlatBuffers compiler, do not modify

/* eslint-disable @typescript-eslint/no-unused-vars, @typescript-eslint/no-explicit-any, @typescript-eslint/no-non-null-assertion */

import * as flatbuffers from 'flatbuffers';

export class MemoryMixerPlayerSubmittedPayload {
  bb: flatbuffers.ByteBuffer|null = null;
  bb_pos = 0;
  __init(i:number, bb:flatbuffers.ByteBuffer):MemoryMixerPlayerSubmittedPayload {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

static getRootAsMemoryMixerPlayerSubmittedPayload(bb:flatbuffers.ByteBuffer, obj?:MemoryMixerPlayerSubmittedPayload):MemoryMixerPlayerSubmittedPayload {
  return (obj || new MemoryMixerPlayerSubmittedPayload()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

static getSizePrefixedRootAsMemoryMixerPlayerSubmittedPayload(bb:flatbuffers.ByteBuffer, obj?:MemoryMixerPlayerSubmittedPayload):MemoryMixerPlayerSubmittedPayload {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new MemoryMixerPlayerSubmittedPayload()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

submitted():boolean {
  const offset = this.bb!.__offset(this.bb_pos, 4);
  return offset ? !!this.bb!.readInt8(this.bb_pos + offset) : false;
}

x():number {
  const offset = this.bb!.__offset(this.bb_pos, 6);
  return offset ? this.bb!.readInt16(this.bb_pos + offset) : 0;
}

y():number {
  const offset = this.bb!.__offset(this.bb_pos, 8);
  return offset ? this.bb!.readInt16(this.bb_pos + offset) : 0;
}

static startMemoryMixerPlayerSubmittedPayload(builder:flatbuffers.Builder) {
  builder.startObject(3);
}

static addSubmitted(builder:flatbuffers.Builder, submitted:boolean) {
  builder.addFieldInt8(0, +submitted, +false);
}

static addX(builder:flatbuffers.Builder, x:number) {
  builder.addFieldInt16(1, x, 0);
}

static addY(builder:flatbuffers.Builder, y:number) {
  builder.addFieldInt16(2, y, 0);
}

static endMemoryMixerPlayerSubmittedPayload(builder:flatbuffers.Builder):flatbuffers.Offset {
  const offset = builder.endObject();
  return offset;
}

static createMemoryMixerPlayerSubmittedPayload(builder:flatbuffers.Builder, submitted:boolean, x:number, y:number):flatbuffers.Offset {
  MemoryMixerPlayerSubmittedPayload.startMemoryMixerPlayerSubmittedPayload(builder);
  MemoryMixerPlayerSubmittedPayload.addSubmitted(builder, submitted);
  MemoryMixerPlayerSubmittedPayload.addX(builder, x);
  MemoryMixerPlayerSubmittedPayload.addY(builder, y);
  return MemoryMixerPlayerSubmittedPayload.endMemoryMixerPlayerSubmittedPayload(builder);
}
}