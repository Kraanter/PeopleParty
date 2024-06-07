// automatically generated by the FlatBuffers compiler, do not modify

/* eslint-disable @typescript-eslint/no-unused-vars, @typescript-eslint/no-explicit-any, @typescript-eslint/no-non-null-assertion */

import * as flatbuffers from 'flatbuffers';

import { MemoryMixerGridRow } from './memory-mixer-grid-row.js';


export class MemoryMixerGridPayload {
  bb: flatbuffers.ByteBuffer|null = null;
  bb_pos = 0;
  __init(i:number, bb:flatbuffers.ByteBuffer):MemoryMixerGridPayload {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

static getRootAsMemoryMixerGridPayload(bb:flatbuffers.ByteBuffer, obj?:MemoryMixerGridPayload):MemoryMixerGridPayload {
  return (obj || new MemoryMixerGridPayload()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

static getSizePrefixedRootAsMemoryMixerGridPayload(bb:flatbuffers.ByteBuffer, obj?:MemoryMixerGridPayload):MemoryMixerGridPayload {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new MemoryMixerGridPayload()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

timeLeft():bigint {
  const offset = this.bb!.__offset(this.bb_pos, 4);
  return offset ? this.bb!.readUint64(this.bb_pos + offset) : BigInt('0');
}

maxOnCard():number {
  const offset = this.bb!.__offset(this.bb_pos, 6);
  return offset ? this.bb!.readInt16(this.bb_pos + offset) : 0;
}

phase():number {
  const offset = this.bb!.__offset(this.bb_pos, 8);
  return offset ? this.bb!.readInt16(this.bb_pos + offset) : 0;
}

round():number {
  const offset = this.bb!.__offset(this.bb_pos, 10);
  return offset ? this.bb!.readInt16(this.bb_pos + offset) : 0;
}

playersLeft():number {
  const offset = this.bb!.__offset(this.bb_pos, 12);
  return offset ? this.bb!.readInt16(this.bb_pos + offset) : 0;
}

names(index: number):string
names(index: number,optionalEncoding:flatbuffers.Encoding):string|Uint8Array
names(index: number,optionalEncoding?:any):string|Uint8Array|null {
  const offset = this.bb!.__offset(this.bb_pos, 14);
  return offset ? this.bb!.__string(this.bb!.__vector(this.bb_pos + offset) + index * 4, optionalEncoding) : null;
}

namesLength():number {
  const offset = this.bb!.__offset(this.bb_pos, 14);
  return offset ? this.bb!.__vector_len(this.bb_pos + offset) : 0;
}

grid(index: number, obj?:MemoryMixerGridRow):MemoryMixerGridRow|null {
  const offset = this.bb!.__offset(this.bb_pos, 16);
  return offset ? (obj || new MemoryMixerGridRow()).__init(this.bb!.__indirect(this.bb!.__vector(this.bb_pos + offset) + index * 4), this.bb!) : null;
}

gridLength():number {
  const offset = this.bb!.__offset(this.bb_pos, 16);
  return offset ? this.bb!.__vector_len(this.bb_pos + offset) : 0;
}

static startMemoryMixerGridPayload(builder:flatbuffers.Builder) {
  builder.startObject(7);
}

static addTimeLeft(builder:flatbuffers.Builder, timeLeft:bigint) {
  builder.addFieldInt64(0, timeLeft, BigInt('0'));
}

static addMaxOnCard(builder:flatbuffers.Builder, maxOnCard:number) {
  builder.addFieldInt16(1, maxOnCard, 0);
}

static addPhase(builder:flatbuffers.Builder, phase:number) {
  builder.addFieldInt16(2, phase, 0);
}

static addRound(builder:flatbuffers.Builder, round:number) {
  builder.addFieldInt16(3, round, 0);
}

static addPlayersLeft(builder:flatbuffers.Builder, playersLeft:number) {
  builder.addFieldInt16(4, playersLeft, 0);
}

static addNames(builder:flatbuffers.Builder, namesOffset:flatbuffers.Offset) {
  builder.addFieldOffset(5, namesOffset, 0);
}

static createNamesVector(builder:flatbuffers.Builder, data:flatbuffers.Offset[]):flatbuffers.Offset {
  builder.startVector(4, data.length, 4);
  for (let i = data.length - 1; i >= 0; i--) {
    builder.addOffset(data[i]!);
  }
  return builder.endVector();
}

static startNamesVector(builder:flatbuffers.Builder, numElems:number) {
  builder.startVector(4, numElems, 4);
}

static addGrid(builder:flatbuffers.Builder, gridOffset:flatbuffers.Offset) {
  builder.addFieldOffset(6, gridOffset, 0);
}

static createGridVector(builder:flatbuffers.Builder, data:flatbuffers.Offset[]):flatbuffers.Offset {
  builder.startVector(4, data.length, 4);
  for (let i = data.length - 1; i >= 0; i--) {
    builder.addOffset(data[i]!);
  }
  return builder.endVector();
}

static startGridVector(builder:flatbuffers.Builder, numElems:number) {
  builder.startVector(4, numElems, 4);
}

static endMemoryMixerGridPayload(builder:flatbuffers.Builder):flatbuffers.Offset {
  const offset = builder.endObject();
  return offset;
}

static createMemoryMixerGridPayload(builder:flatbuffers.Builder, timeLeft:bigint, maxOnCard:number, phase:number, round:number, playersLeft:number, namesOffset:flatbuffers.Offset, gridOffset:flatbuffers.Offset):flatbuffers.Offset {
  MemoryMixerGridPayload.startMemoryMixerGridPayload(builder);
  MemoryMixerGridPayload.addTimeLeft(builder, timeLeft);
  MemoryMixerGridPayload.addMaxOnCard(builder, maxOnCard);
  MemoryMixerGridPayload.addPhase(builder, phase);
  MemoryMixerGridPayload.addRound(builder, round);
  MemoryMixerGridPayload.addPlayersLeft(builder, playersLeft);
  MemoryMixerGridPayload.addNames(builder, namesOffset);
  MemoryMixerGridPayload.addGrid(builder, gridOffset);
  return MemoryMixerGridPayload.endMemoryMixerGridPayload(builder);
}
}