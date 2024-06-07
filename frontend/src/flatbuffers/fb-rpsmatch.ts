// automatically generated by the FlatBuffers compiler, do not modify

/* eslint-disable @typescript-eslint/no-unused-vars, @typescript-eslint/no-explicit-any, @typescript-eslint/no-non-null-assertion */

import * as flatbuffers from 'flatbuffers';

export class FB_RPSMatch {
  bb: flatbuffers.ByteBuffer|null = null;
  bb_pos = 0;
  __init(i:number, bb:flatbuffers.ByteBuffer):FB_RPSMatch {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

static getRootAsFB_RPSMatch(bb:flatbuffers.ByteBuffer, obj?:FB_RPSMatch):FB_RPSMatch {
  return (obj || new FB_RPSMatch()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

static getSizePrefixedRootAsFB_RPSMatch(bb:flatbuffers.ByteBuffer, obj?:FB_RPSMatch):FB_RPSMatch {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new FB_RPSMatch()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
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

static startFB_RPSMatch(builder:flatbuffers.Builder) {
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

static endFB_RPSMatch(builder:flatbuffers.Builder):flatbuffers.Offset {
  const offset = builder.endObject();
  return offset;
}

static createFB_RPSMatch(builder:flatbuffers.Builder, player1Offset:flatbuffers.Offset, player2Offset:flatbuffers.Offset, winnerOffset:flatbuffers.Offset):flatbuffers.Offset {
  FB_RPSMatch.startFB_RPSMatch(builder);
  FB_RPSMatch.addPlayer1(builder, player1Offset);
  FB_RPSMatch.addPlayer2(builder, player2Offset);
  FB_RPSMatch.addWinner(builder, winnerOffset);
  return FB_RPSMatch.endFB_RPSMatch(builder);
}
}
