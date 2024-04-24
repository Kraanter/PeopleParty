// automatically generated by the FlatBuffers compiler, do not modify

/* eslint-disable @typescript-eslint/no-unused-vars, @typescript-eslint/no-explicit-any, @typescript-eslint/no-non-null-assertion */

import { CountingClientData } from './counting-client-data.js';
import { CountingGameState } from './counting-game-state.js';


export enum GameStatePayload {
  NONE = 0,
  CountingClientData = 1,
  CountingGameState = 2
}

export function unionToGameStatePayload(
  type: GameStatePayload,
  accessor: (obj:CountingClientData|CountingGameState) => CountingClientData|CountingGameState|null
): CountingClientData|CountingGameState|null {
  switch(GameStatePayload[type]) {
    case 'NONE': return null;
    case 'CountingClientData': return accessor(new CountingClientData())! as CountingClientData;
    case 'CountingGameState': return accessor(new CountingGameState())! as CountingGameState;
    default: return null;
  }
}

export function unionListToGameStatePayload(
  type: GameStatePayload,
  accessor: (index: number, obj:CountingClientData|CountingGameState) => CountingClientData|CountingGameState|null,
  index: number
): CountingClientData|CountingGameState|null {
  switch(GameStatePayload[type]) {
    case 'NONE': return null;
    case 'CountingClientData': return accessor(index, new CountingClientData())! as CountingClientData;
    case 'CountingGameState': return accessor(index, new CountingGameState())! as CountingGameState;
    default: return null;
  }
}