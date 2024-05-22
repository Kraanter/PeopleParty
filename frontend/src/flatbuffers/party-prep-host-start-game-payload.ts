// automatically generated by the FlatBuffers compiler, do not modify

/* eslint-disable @typescript-eslint/no-unused-vars, @typescript-eslint/no-explicit-any, @typescript-eslint/no-non-null-assertion */

import * as flatbuffers from 'flatbuffers';

export class PartyPrepHostStartGamePayload {
  bb: flatbuffers.ByteBuffer|null = null;
  bb_pos = 0;
  __init(i:number, bb:flatbuffers.ByteBuffer):PartyPrepHostStartGamePayload {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

static getRootAsPartyPrepHostStartGamePayload(bb:flatbuffers.ByteBuffer, obj?:PartyPrepHostStartGamePayload):PartyPrepHostStartGamePayload {
  return (obj || new PartyPrepHostStartGamePayload()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

static getSizePrefixedRootAsPartyPrepHostStartGamePayload(bb:flatbuffers.ByteBuffer, obj?:PartyPrepHostStartGamePayload):PartyPrepHostStartGamePayload {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new PartyPrepHostStartGamePayload()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

startGame():boolean {
  const offset = this.bb!.__offset(this.bb_pos, 4);
  return offset ? !!this.bb!.readInt8(this.bb_pos + offset) : false;
}

static startPartyPrepHostStartGamePayload(builder:flatbuffers.Builder) {
  builder.startObject(1);
}

static addStartGame(builder:flatbuffers.Builder, startGame:boolean) {
  builder.addFieldInt8(0, +startGame, +false);
}

static endPartyPrepHostStartGamePayload(builder:flatbuffers.Builder):flatbuffers.Offset {
  const offset = builder.endObject();
  return offset;
}

static createPartyPrepHostStartGamePayload(builder:flatbuffers.Builder, startGame:boolean):flatbuffers.Offset {
  PartyPrepHostStartGamePayload.startPartyPrepHostStartGamePayload(builder);
  PartyPrepHostStartGamePayload.addStartGame(builder, startGame);
  return PartyPrepHostStartGamePayload.endPartyPrepHostStartGamePayload(builder);
}
}