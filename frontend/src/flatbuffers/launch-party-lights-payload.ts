// automatically generated by the FlatBuffers compiler, do not modify

/* eslint-disable @typescript-eslint/no-unused-vars, @typescript-eslint/no-explicit-any, @typescript-eslint/no-non-null-assertion */

import * as flatbuffers from 'flatbuffers';

export class LaunchPartyLightsPayload {
  bb: flatbuffers.ByteBuffer|null = null;
  bb_pos = 0;
  __init(i:number, bb:flatbuffers.ByteBuffer):LaunchPartyLightsPayload {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

static getRootAsLaunchPartyLightsPayload(bb:flatbuffers.ByteBuffer, obj?:LaunchPartyLightsPayload):LaunchPartyLightsPayload {
  return (obj || new LaunchPartyLightsPayload()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

static getSizePrefixedRootAsLaunchPartyLightsPayload(bb:flatbuffers.ByteBuffer, obj?:LaunchPartyLightsPayload):LaunchPartyLightsPayload {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new LaunchPartyLightsPayload()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

pratice():boolean {
  const offset = this.bb!.__offset(this.bb_pos, 4);
  return offset ? !!this.bb!.readInt8(this.bb_pos + offset) : false;
}

lights():number {
  const offset = this.bb!.__offset(this.bb_pos, 6);
  return offset ? this.bb!.readInt16(this.bb_pos + offset) : 0;
}

static startLaunchPartyLightsPayload(builder:flatbuffers.Builder) {
  builder.startObject(2);
}

static addPratice(builder:flatbuffers.Builder, pratice:boolean) {
  builder.addFieldInt8(0, +pratice, +false);
}

static addLights(builder:flatbuffers.Builder, lights:number) {
  builder.addFieldInt16(1, lights, 0);
}

static endLaunchPartyLightsPayload(builder:flatbuffers.Builder):flatbuffers.Offset {
  const offset = builder.endObject();
  return offset;
}

static createLaunchPartyLightsPayload(builder:flatbuffers.Builder, pratice:boolean, lights:number):flatbuffers.Offset {
  LaunchPartyLightsPayload.startLaunchPartyLightsPayload(builder);
  LaunchPartyLightsPayload.addPratice(builder, pratice);
  LaunchPartyLightsPayload.addLights(builder, lights);
  return LaunchPartyLightsPayload.endLaunchPartyLightsPayload(builder);
}
}
