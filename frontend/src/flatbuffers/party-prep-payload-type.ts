// automatically generated by the FlatBuffers compiler, do not modify

/* eslint-disable @typescript-eslint/no-unused-vars, @typescript-eslint/no-explicit-any, @typescript-eslint/no-non-null-assertion */

import * as flatbuffers from 'flatbuffers';

import { PartyPrepPayload, unionToPartyPrepPayload, unionListToPartyPrepPayload } from './party-prep-payload.js';
import { PartyPrepType } from './party-prep-type.js';


export class PartyPrepPayloadType {
  bb: flatbuffers.ByteBuffer|null = null;
  bb_pos = 0;
  __init(i:number, bb:flatbuffers.ByteBuffer):PartyPrepPayloadType {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

static getRootAsPartyPrepPayloadType(bb:flatbuffers.ByteBuffer, obj?:PartyPrepPayloadType):PartyPrepPayloadType {
  return (obj || new PartyPrepPayloadType()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

static getSizePrefixedRootAsPartyPrepPayloadType(bb:flatbuffers.ByteBuffer, obj?:PartyPrepPayloadType):PartyPrepPayloadType {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new PartyPrepPayloadType()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

partypreptype():PartyPrepType {
  const offset = this.bb!.__offset(this.bb_pos, 4);
  return offset ? this.bb!.readInt8(this.bb_pos + offset) : PartyPrepType.PartyPrepHostInformation;
}

partypreppayloadType():PartyPrepPayload {
  const offset = this.bb!.__offset(this.bb_pos, 6);
  return offset ? this.bb!.readUint8(this.bb_pos + offset) : PartyPrepPayload.NONE;
}

partypreppayload<T extends flatbuffers.Table>(obj:any):any|null {
  const offset = this.bb!.__offset(this.bb_pos, 8);
  return offset ? this.bb!.__union(obj, this.bb_pos + offset) : null;
}

static startPartyPrepPayloadType(builder:flatbuffers.Builder) {
  builder.startObject(3);
}

static addPartypreptype(builder:flatbuffers.Builder, partypreptype:PartyPrepType) {
  builder.addFieldInt8(0, partypreptype, PartyPrepType.PartyPrepHostInformation);
}

static addPartypreppayloadType(builder:flatbuffers.Builder, partypreppayloadType:PartyPrepPayload) {
  builder.addFieldInt8(1, partypreppayloadType, PartyPrepPayload.NONE);
}

static addPartypreppayload(builder:flatbuffers.Builder, partypreppayloadOffset:flatbuffers.Offset) {
  builder.addFieldOffset(2, partypreppayloadOffset, 0);
}

static endPartyPrepPayloadType(builder:flatbuffers.Builder):flatbuffers.Offset {
  const offset = builder.endObject();
  return offset;
}

static createPartyPrepPayloadType(builder:flatbuffers.Builder, partypreptype:PartyPrepType, partypreppayloadType:PartyPrepPayload, partypreppayloadOffset:flatbuffers.Offset):flatbuffers.Offset {
  PartyPrepPayloadType.startPartyPrepPayloadType(builder);
  PartyPrepPayloadType.addPartypreptype(builder, partypreptype);
  PartyPrepPayloadType.addPartypreppayloadType(builder, partypreppayloadType);
  PartyPrepPayloadType.addPartypreppayload(builder, partypreppayloadOffset);
  return PartyPrepPayloadType.endPartyPrepPayloadType(builder);
}
}
