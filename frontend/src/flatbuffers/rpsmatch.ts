// automatically generated by the FlatBuffers compiler, do not modify

/* eslint-disable @typescript-eslint/no-unused-vars, @typescript-eslint/no-explicit-any, @typescript-eslint/no-non-null-assertion */

import * as flatbuffers from 'flatbuffers';

export class RPSMatch {
  bb: flatbuffers.ByteBuffer|null = null;
  bb_pos = 0;
  __init(i:number, bb:flatbuffers.ByteBuffer):RPSMatch {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

static getRootAsRPSMatch(bb:flatbuffers.ByteBuffer, obj?:RPSMatch):RPSMatch {
  return (obj || new RPSMatch()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

static getSizePrefixedRootAsRPSMatch(bb:flatbuffers.ByteBuffer, obj?:RPSMatch):RPSMatch {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new RPSMatch()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

player1():string|null
player1(optionalEncoding:flatbuffers.Encoding):string|Uint8Array|null
player1(optionalEncoding?:any):string|Uint8Array|null {
  const offset = this.bb!.__offset(this.bb_pos, 4);
  return offset ? this.bb!.__string(this.bb_pos + offset, optionalEncoding) : null;
}

player2():string|null
player2(optionalEncoding:flatbuffers.Encoding):string|Uint8Array|null
player2(optionalEncoding?:any):string|Uint8Array|null {
  const offset = this.bb!.__offset(this.bb_pos, 6);
  return offset ? this.bb!.__string(this.bb_pos + offset, optionalEncoding) : null;
}

winner():string|null
winner(optionalEncoding:flatbuffers.Encoding):string|Uint8Array|null
winner(optionalEncoding?:any):string|Uint8Array|null {
  const offset = this.bb!.__offset(this.bb_pos, 8);
  return offset ? this.bb!.__string(this.bb_pos + offset, optionalEncoding) : null;
}

static startRPSMatch(builder:flatbuffers.Builder) {
  builder.startObject(3);
}

static addPlayer1(builder:flatbuffers.Builder, player1Offset:flatbuffers.Offset) {
  builder.addFieldOffset(0, player1Offset, 0);
}

static addPlayer2(builder:flatbuffers.Builder, player2Offset:flatbuffers.Offset) {
  builder.addFieldOffset(1, player2Offset, 0);
}

static addWinner(builder:flatbuffers.Builder, winnerOffset:flatbuffers.Offset) {
  builder.addFieldOffset(2, winnerOffset, 0);
}

static endRPSMatch(builder:flatbuffers.Builder):flatbuffers.Offset {
  const offset = builder.endObject();
  return offset;
}

static createRPSMatch(builder:flatbuffers.Builder, player1Offset:flatbuffers.Offset, player2Offset:flatbuffers.Offset, winnerOffset:flatbuffers.Offset):flatbuffers.Offset {
  RPSMatch.startRPSMatch(builder);
  RPSMatch.addPlayer1(builder, player1Offset);
  RPSMatch.addPlayer2(builder, player2Offset);
  RPSMatch.addWinner(builder, winnerOffset);
  return RPSMatch.endRPSMatch(builder);
}
}