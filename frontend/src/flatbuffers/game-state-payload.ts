// automatically generated by the FlatBuffers compiler, do not modify

/* eslint-disable @typescript-eslint/no-unused-vars, @typescript-eslint/no-explicit-any, @typescript-eslint/no-non-null-assertion */

import { CountingClientDataPayload } from './counting-client-data-payload.js'
import { CountingGameStatePayload } from './counting-game-state-payload.js'

export enum GameStatePayload {
  NONE = 0,
  CountingClientDataPayload = 1,
  CountingGameStatePayload = 2
}

export function unionToGameStatePayload(
  type: GameStatePayload,
  accessor: (
    obj: CountingClientDataPayload | CountingGameStatePayload
  ) => CountingClientDataPayload | CountingGameStatePayload | null
): CountingClientDataPayload | CountingGameStatePayload | null {
  switch (GameStatePayload[type]) {
    case 'NONE':
      return null
    case 'CountingClientDataPayload':
      return accessor(new CountingClientDataPayload())! as CountingClientDataPayload
    case 'CountingGameStatePayload':
      return accessor(new CountingGameStatePayload())! as CountingGameStatePayload
    default:
      return null
  }
}

export function unionListToGameStatePayload(
  type: GameStatePayload,
  accessor: (
    index: number,
    obj: CountingClientDataPayload | CountingGameStatePayload
  ) => CountingClientDataPayload | CountingGameStatePayload | null,
  index: number
): CountingClientDataPayload | CountingGameStatePayload | null {
  switch (GameStatePayload[type]) {
    case 'NONE':
      return null
    case 'CountingClientDataPayload':
      return accessor(index, new CountingClientDataPayload())! as CountingClientDataPayload
    case 'CountingGameStatePayload':
      return accessor(index, new CountingGameStatePayload())! as CountingGameStatePayload
    default:
      return null
  }
}
