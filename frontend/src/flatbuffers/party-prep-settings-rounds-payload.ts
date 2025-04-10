// automatically generated by the FlatBuffers compiler, do not modify

/* eslint-disable @typescript-eslint/no-unused-vars, @typescript-eslint/no-explicit-any, @typescript-eslint/no-non-null-assertion */

import * as flatbuffers from 'flatbuffers';

export class PartyPrepSettingsRoundsPayload {
  bb: flatbuffers.ByteBuffer|null = null;
  bb_pos = 0;
  __init(i:number, bb:flatbuffers.ByteBuffer):PartyPrepSettingsRoundsPayload {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

static getRootAsPartyPrepSettingsRoundsPayload(bb:flatbuffers.ByteBuffer, obj?:PartyPrepSettingsRoundsPayload):PartyPrepSettingsRoundsPayload {
  return (obj || new PartyPrepSettingsRoundsPayload()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

static getSizePrefixedRootAsPartyPrepSettingsRoundsPayload(bb:flatbuffers.ByteBuffer, obj?:PartyPrepSettingsRoundsPayload):PartyPrepSettingsRoundsPayload {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new PartyPrepSettingsRoundsPayload()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

newNumberOfRounds():bigint {
  const offset = this.bb!.__offset(this.bb_pos, 4);
  return offset ? this.bb!.readInt64(this.bb_pos + offset) : BigInt('0');
}

static startPartyPrepSettingsRoundsPayload(builder:flatbuffers.Builder) {
  builder.startObject(1);
}

static addNewNumberOfRounds(builder:flatbuffers.Builder, newNumberOfRounds:bigint) {
  builder.addFieldInt64(0, newNumberOfRounds, BigInt('0'));
}

static endPartyPrepSettingsRoundsPayload(builder:flatbuffers.Builder):flatbuffers.Offset {
  const offset = builder.endObject();
  return offset;
}

static createPartyPrepSettingsRoundsPayload(builder:flatbuffers.Builder, newNumberOfRounds:bigint):flatbuffers.Offset {
  PartyPrepSettingsRoundsPayload.startPartyPrepSettingsRoundsPayload(builder);
  PartyPrepSettingsRoundsPayload.addNewNumberOfRounds(builder, newNumberOfRounds);
  return PartyPrepSettingsRoundsPayload.endPartyPrepSettingsRoundsPayload(builder);
}
}
