// automatically generated by the FlatBuffers compiler, do not modify

/* eslint-disable @typescript-eslint/no-unused-vars, @typescript-eslint/no-explicit-any, @typescript-eslint/no-non-null-assertion */

import * as flatbuffers from 'flatbuffers';

export class BusinessBailoutResultPayload {
  bb: flatbuffers.ByteBuffer|null = null;
  bb_pos = 0;
  __init(i:number, bb:flatbuffers.ByteBuffer):BusinessBailoutResultPayload {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

static getRootAsBusinessBailoutResultPayload(bb:flatbuffers.ByteBuffer, obj?:BusinessBailoutResultPayload):BusinessBailoutResultPayload {
  return (obj || new BusinessBailoutResultPayload()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

static getSizePrefixedRootAsBusinessBailoutResultPayload(bb:flatbuffers.ByteBuffer, obj?:BusinessBailoutResultPayload):BusinessBailoutResultPayload {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new BusinessBailoutResultPayload()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

value():number {
  const offset = this.bb!.__offset(this.bb_pos, 4);
  return offset ? this.bb!.readInt32(this.bb_pos + offset) : 0;
}

static startBusinessBailoutResultPayload(builder:flatbuffers.Builder) {
  builder.startObject(1);
}

static addValue(builder:flatbuffers.Builder, value:number) {
  builder.addFieldInt32(0, value, 0);
}

static endBusinessBailoutResultPayload(builder:flatbuffers.Builder):flatbuffers.Offset {
  const offset = builder.endObject();
  return offset;
}

static createBusinessBailoutResultPayload(builder:flatbuffers.Builder, value:number):flatbuffers.Offset {
  BusinessBailoutResultPayload.startBusinessBailoutResultPayload(builder);
  BusinessBailoutResultPayload.addValue(builder, value);
  return BusinessBailoutResultPayload.endBusinessBailoutResultPayload(builder);
}
}