// automatically generated by the FlatBuffers compiler, do not modify

/* eslint-disable @typescript-eslint/no-unused-vars, @typescript-eslint/no-explicit-any, @typescript-eslint/no-non-null-assertion */

import * as flatbuffers from 'flatbuffers';

export class FBMiniGameSetting {
  bb: flatbuffers.ByteBuffer|null = null;
  bb_pos = 0;
  __init(i:number, bb:flatbuffers.ByteBuffer):FBMiniGameSetting {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

static getRootAsFBMiniGameSetting(bb:flatbuffers.ByteBuffer, obj?:FBMiniGameSetting):FBMiniGameSetting {
  return (obj || new FBMiniGameSetting()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

static getSizePrefixedRootAsFBMiniGameSetting(bb:flatbuffers.ByteBuffer, obj?:FBMiniGameSetting):FBMiniGameSetting {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new FBMiniGameSetting()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

name():string|null
name(optionalEncoding:flatbuffers.Encoding):string|Uint8Array|null
name(optionalEncoding?:any):string|Uint8Array|null {
  const offset = this.bb!.__offset(this.bb_pos, 4);
  return offset ? this.bb!.__string(this.bb_pos + offset, optionalEncoding) : null;
}

enabled():boolean {
  const offset = this.bb!.__offset(this.bb_pos, 6);
  return offset ? !!this.bb!.readInt8(this.bb_pos + offset) : false;
}

static startFBMiniGameSetting(builder:flatbuffers.Builder) {
  builder.startObject(2);
}

static addName(builder:flatbuffers.Builder, nameOffset:flatbuffers.Offset) {
  builder.addFieldOffset(0, nameOffset, 0);
}

static addEnabled(builder:flatbuffers.Builder, enabled:boolean) {
  builder.addFieldInt8(1, +enabled, +false);
}

static endFBMiniGameSetting(builder:flatbuffers.Builder):flatbuffers.Offset {
  const offset = builder.endObject();
  return offset;
}

static createFBMiniGameSetting(builder:flatbuffers.Builder, nameOffset:flatbuffers.Offset, enabled:boolean):flatbuffers.Offset {
  FBMiniGameSetting.startFBMiniGameSetting(builder);
  FBMiniGameSetting.addName(builder, nameOffset);
  FBMiniGameSetting.addEnabled(builder, enabled);
  return FBMiniGameSetting.endFBMiniGameSetting(builder);
}
}
