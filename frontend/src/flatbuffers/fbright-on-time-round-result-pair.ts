// automatically generated by the FlatBuffers compiler, do not modify

/* eslint-disable @typescript-eslint/no-unused-vars, @typescript-eslint/no-explicit-any, @typescript-eslint/no-non-null-assertion */

import * as flatbuffers from 'flatbuffers';

export class FBRightOnTimeRoundResultPair {
  bb: flatbuffers.ByteBuffer|null = null;
  bb_pos = 0;
  __init(i:number, bb:flatbuffers.ByteBuffer):FBRightOnTimeRoundResultPair {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

static getRootAsFBRightOnTimeRoundResultPair(bb:flatbuffers.ByteBuffer, obj?:FBRightOnTimeRoundResultPair):FBRightOnTimeRoundResultPair {
  return (obj || new FBRightOnTimeRoundResultPair()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

static getSizePrefixedRootAsFBRightOnTimeRoundResultPair(bb:flatbuffers.ByteBuffer, obj?:FBRightOnTimeRoundResultPair):FBRightOnTimeRoundResultPair {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new FBRightOnTimeRoundResultPair()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

name():string|null
name(optionalEncoding:flatbuffers.Encoding):string|Uint8Array|null
name(optionalEncoding?:any):string|Uint8Array|null {
  const offset = this.bb!.__offset(this.bb_pos, 4);
  return offset ? this.bb!.__string(this.bb_pos + offset, optionalEncoding) : null;
}

diff():bigint {
  const offset = this.bb!.__offset(this.bb_pos, 6);
  return offset ? this.bb!.readInt64(this.bb_pos + offset) : BigInt('0');
}

static startFBRightOnTimeRoundResultPair(builder:flatbuffers.Builder) {
  builder.startObject(2);
}

static addName(builder:flatbuffers.Builder, nameOffset:flatbuffers.Offset) {
  builder.addFieldOffset(0, nameOffset, 0);
}

static addDiff(builder:flatbuffers.Builder, diff:bigint) {
  builder.addFieldInt64(1, diff, BigInt('0'));
}

static endFBRightOnTimeRoundResultPair(builder:flatbuffers.Builder):flatbuffers.Offset {
  const offset = builder.endObject();
  return offset;
}

static createFBRightOnTimeRoundResultPair(builder:flatbuffers.Builder, nameOffset:flatbuffers.Offset, diff:bigint):flatbuffers.Offset {
  FBRightOnTimeRoundResultPair.startFBRightOnTimeRoundResultPair(builder);
  FBRightOnTimeRoundResultPair.addName(builder, nameOffset);
  FBRightOnTimeRoundResultPair.addDiff(builder, diff);
  return FBRightOnTimeRoundResultPair.endFBRightOnTimeRoundResultPair(builder);
}
}
