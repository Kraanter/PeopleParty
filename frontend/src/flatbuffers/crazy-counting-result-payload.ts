// automatically generated by the FlatBuffers compiler, do not modify

/* eslint-disable @typescript-eslint/no-unused-vars, @typescript-eslint/no-explicit-any, @typescript-eslint/no-non-null-assertion */

import * as flatbuffers from 'flatbuffers';

import { FBCrazyCountingResultPair } from './fbcrazy-counting-result-pair.js';


export class CrazyCountingResultPayload {
  bb: flatbuffers.ByteBuffer|null = null;
  bb_pos = 0;
  __init(i:number, bb:flatbuffers.ByteBuffer):CrazyCountingResultPayload {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

static getRootAsCrazyCountingResultPayload(bb:flatbuffers.ByteBuffer, obj?:CrazyCountingResultPayload):CrazyCountingResultPayload {
  return (obj || new CrazyCountingResultPayload()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

static getSizePrefixedRootAsCrazyCountingResultPayload(bb:flatbuffers.ByteBuffer, obj?:CrazyCountingResultPayload):CrazyCountingResultPayload {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new CrazyCountingResultPayload()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

correctAnswer():number {
  const offset = this.bb!.__offset(this.bb_pos, 4);
  return offset ? this.bb!.readInt16(this.bb_pos + offset) : 0;
}

results(index: number, obj?:FBCrazyCountingResultPair):FBCrazyCountingResultPair|null {
  const offset = this.bb!.__offset(this.bb_pos, 6);
  return offset ? (obj || new FBCrazyCountingResultPair()).__init(this.bb!.__indirect(this.bb!.__vector(this.bb_pos + offset) + index * 4), this.bb!) : null;
}

resultsLength():number {
  const offset = this.bb!.__offset(this.bb_pos, 6);
  return offset ? this.bb!.__vector_len(this.bb_pos + offset) : 0;
}

static startCrazyCountingResultPayload(builder:flatbuffers.Builder) {
  builder.startObject(2);
}

static addCorrectAnswer(builder:flatbuffers.Builder, correctAnswer:number) {
  builder.addFieldInt16(0, correctAnswer, 0);
}

static addResults(builder:flatbuffers.Builder, resultsOffset:flatbuffers.Offset) {
  builder.addFieldOffset(1, resultsOffset, 0);
}

static createResultsVector(builder:flatbuffers.Builder, data:flatbuffers.Offset[]):flatbuffers.Offset {
  builder.startVector(4, data.length, 4);
  for (let i = data.length - 1; i >= 0; i--) {
    builder.addOffset(data[i]!);
  }
  return builder.endVector();
}

static startResultsVector(builder:flatbuffers.Builder, numElems:number) {
  builder.startVector(4, numElems, 4);
}

static endCrazyCountingResultPayload(builder:flatbuffers.Builder):flatbuffers.Offset {
  const offset = builder.endObject();
  return offset;
}

static createCrazyCountingResultPayload(builder:flatbuffers.Builder, correctAnswer:number, resultsOffset:flatbuffers.Offset):flatbuffers.Offset {
  CrazyCountingResultPayload.startCrazyCountingResultPayload(builder);
  CrazyCountingResultPayload.addCorrectAnswer(builder, correctAnswer);
  CrazyCountingResultPayload.addResults(builder, resultsOffset);
  return CrazyCountingResultPayload.endCrazyCountingResultPayload(builder);
}
}
