// automatically generated by the FlatBuffers compiler, do not modify

/* eslint-disable @typescript-eslint/no-unused-vars, @typescript-eslint/no-explicit-any, @typescript-eslint/no-non-null-assertion */

import * as flatbuffers from 'flatbuffers';

export class PartyPrepSettingsMiniGamePayload {
  bb: flatbuffers.ByteBuffer|null = null;
  bb_pos = 0;
  __init(i:number, bb:flatbuffers.ByteBuffer):PartyPrepSettingsMiniGamePayload {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

static getRootAsPartyPrepSettingsMiniGamePayload(bb:flatbuffers.ByteBuffer, obj?:PartyPrepSettingsMiniGamePayload):PartyPrepSettingsMiniGamePayload {
  return (obj || new PartyPrepSettingsMiniGamePayload()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

static getSizePrefixedRootAsPartyPrepSettingsMiniGamePayload(bb:flatbuffers.ByteBuffer, obj?:PartyPrepSettingsMiniGamePayload):PartyPrepSettingsMiniGamePayload {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new PartyPrepSettingsMiniGamePayload()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

name():string|null
name(optionalEncoding:flatbuffers.Encoding):string|Uint8Array|null
name(optionalEncoding?:any):string|Uint8Array|null {
  const offset = this.bb!.__offset(this.bb_pos, 4);
  return offset ? this.bb!.__string(this.bb_pos + offset, optionalEncoding) : null;
}

enable():boolean {
  const offset = this.bb!.__offset(this.bb_pos, 6);
  return offset ? !!this.bb!.readInt8(this.bb_pos + offset) : false;
}

static startPartyPrepSettingsMiniGamePayload(builder:flatbuffers.Builder) {
  builder.startObject(2);
}

static addName(builder:flatbuffers.Builder, nameOffset:flatbuffers.Offset) {
  builder.addFieldOffset(0, nameOffset, 0);
}

static addEnable(builder:flatbuffers.Builder, enable:boolean) {
  builder.addFieldInt8(1, +enable, +false);
}

static endPartyPrepSettingsMiniGamePayload(builder:flatbuffers.Builder):flatbuffers.Offset {
  const offset = builder.endObject();
  return offset;
}

static createPartyPrepSettingsMiniGamePayload(builder:flatbuffers.Builder, nameOffset:flatbuffers.Offset, enable:boolean):flatbuffers.Offset {
  PartyPrepSettingsMiniGamePayload.startPartyPrepSettingsMiniGamePayload(builder);
  PartyPrepSettingsMiniGamePayload.addName(builder, nameOffset);
  PartyPrepSettingsMiniGamePayload.addEnable(builder, enable);
  return PartyPrepSettingsMiniGamePayload.endPartyPrepSettingsMiniGamePayload(builder);
}
}
