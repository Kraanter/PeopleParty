// automatically generated by the FlatBuffers compiler, do not modify

/* eslint-disable @typescript-eslint/no-unused-vars, @typescript-eslint/no-explicit-any, @typescript-eslint/no-non-null-assertion */

import * as flatbuffers from 'flatbuffers';

export class LaunchPartyPlayerInputPayload {
  bb: flatbuffers.ByteBuffer|null = null;
  bb_pos = 0;
  __init(i:number, bb:flatbuffers.ByteBuffer):LaunchPartyPlayerInputPayload {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

static getRootAsLaunchPartyPlayerInputPayload(bb:flatbuffers.ByteBuffer, obj?:LaunchPartyPlayerInputPayload):LaunchPartyPlayerInputPayload {
  return (obj || new LaunchPartyPlayerInputPayload()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

static getSizePrefixedRootAsLaunchPartyPlayerInputPayload(bb:flatbuffers.ByteBuffer, obj?:LaunchPartyPlayerInputPayload):LaunchPartyPlayerInputPayload {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new LaunchPartyPlayerInputPayload()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

pressed():boolean {
  const offset = this.bb!.__offset(this.bb_pos, 4);
  return offset ? !!this.bb!.readInt8(this.bb_pos + offset) : false;
}

static startLaunchPartyPlayerInputPayload(builder:flatbuffers.Builder) {
  builder.startObject(1);
}

static addPressed(builder:flatbuffers.Builder, pressed:boolean) {
  builder.addFieldInt8(0, +pressed, +false);
}

static endLaunchPartyPlayerInputPayload(builder:flatbuffers.Builder):flatbuffers.Offset {
  const offset = builder.endObject();
  return offset;
}

static createLaunchPartyPlayerInputPayload(builder:flatbuffers.Builder, pressed:boolean):flatbuffers.Offset {
  LaunchPartyPlayerInputPayload.startLaunchPartyPlayerInputPayload(builder);
  LaunchPartyPlayerInputPayload.addPressed(builder, pressed);
  return LaunchPartyPlayerInputPayload.endLaunchPartyPlayerInputPayload(builder);
}
}
