// automatically generated by the FlatBuffers compiler, do not modify

/* eslint-disable @typescript-eslint/no-unused-vars, @typescript-eslint/no-explicit-any, @typescript-eslint/no-non-null-assertion */

import { PartyPrepHostInformationPayload } from './party-prep-host-information-payload.js';
import { PartyPrepHostStartGamePayload } from './party-prep-host-start-game-payload.js';
import { PartyPrepPlayerInformationPayload } from './party-prep-player-information-payload.js';
import { PartyPrepSettingsInformationPayload } from './party-prep-settings-information-payload.js';
import { PartyPrepSettingsMiniGamePayload } from './party-prep-settings-mini-game-payload.js';
import { PartyPrepSettingsRoundsPayload } from './party-prep-settings-rounds-payload.js';


export enum PartyPrepPayload {
  NONE = 0,
  PartyPrepHostInformationPayload = 1,
  PartyPrepHostStartGamePayload = 2,
  PartyPrepPlayerInformationPayload = 3,
  PartyPrepSettingsInformationPayload = 4,
  PartyPrepSettingsRoundsPayload = 5,
  PartyPrepSettingsMiniGamePayload = 6
}

export function unionToPartyPrepPayload(
  type: PartyPrepPayload,
  accessor: (obj:PartyPrepHostInformationPayload|PartyPrepHostStartGamePayload|PartyPrepPlayerInformationPayload|PartyPrepSettingsInformationPayload|PartyPrepSettingsMiniGamePayload|PartyPrepSettingsRoundsPayload) => PartyPrepHostInformationPayload|PartyPrepHostStartGamePayload|PartyPrepPlayerInformationPayload|PartyPrepSettingsInformationPayload|PartyPrepSettingsMiniGamePayload|PartyPrepSettingsRoundsPayload|null
): PartyPrepHostInformationPayload|PartyPrepHostStartGamePayload|PartyPrepPlayerInformationPayload|PartyPrepSettingsInformationPayload|PartyPrepSettingsMiniGamePayload|PartyPrepSettingsRoundsPayload|null {
  switch(PartyPrepPayload[type]) {
    case 'NONE': return null; 
    case 'PartyPrepHostInformationPayload': return accessor(new PartyPrepHostInformationPayload())! as PartyPrepHostInformationPayload;
    case 'PartyPrepHostStartGamePayload': return accessor(new PartyPrepHostStartGamePayload())! as PartyPrepHostStartGamePayload;
    case 'PartyPrepPlayerInformationPayload': return accessor(new PartyPrepPlayerInformationPayload())! as PartyPrepPlayerInformationPayload;
    case 'PartyPrepSettingsInformationPayload': return accessor(new PartyPrepSettingsInformationPayload())! as PartyPrepSettingsInformationPayload;
    case 'PartyPrepSettingsRoundsPayload': return accessor(new PartyPrepSettingsRoundsPayload())! as PartyPrepSettingsRoundsPayload;
    case 'PartyPrepSettingsMiniGamePayload': return accessor(new PartyPrepSettingsMiniGamePayload())! as PartyPrepSettingsMiniGamePayload;
    default: return null;
  }
}

export function unionListToPartyPrepPayload(
  type: PartyPrepPayload, 
  accessor: (index: number, obj:PartyPrepHostInformationPayload|PartyPrepHostStartGamePayload|PartyPrepPlayerInformationPayload|PartyPrepSettingsInformationPayload|PartyPrepSettingsMiniGamePayload|PartyPrepSettingsRoundsPayload) => PartyPrepHostInformationPayload|PartyPrepHostStartGamePayload|PartyPrepPlayerInformationPayload|PartyPrepSettingsInformationPayload|PartyPrepSettingsMiniGamePayload|PartyPrepSettingsRoundsPayload|null, 
  index: number
): PartyPrepHostInformationPayload|PartyPrepHostStartGamePayload|PartyPrepPlayerInformationPayload|PartyPrepSettingsInformationPayload|PartyPrepSettingsMiniGamePayload|PartyPrepSettingsRoundsPayload|null {
  switch(PartyPrepPayload[type]) {
    case 'NONE': return null; 
    case 'PartyPrepHostInformationPayload': return accessor(index, new PartyPrepHostInformationPayload())! as PartyPrepHostInformationPayload;
    case 'PartyPrepHostStartGamePayload': return accessor(index, new PartyPrepHostStartGamePayload())! as PartyPrepHostStartGamePayload;
    case 'PartyPrepPlayerInformationPayload': return accessor(index, new PartyPrepPlayerInformationPayload())! as PartyPrepPlayerInformationPayload;
    case 'PartyPrepSettingsInformationPayload': return accessor(index, new PartyPrepSettingsInformationPayload())! as PartyPrepSettingsInformationPayload;
    case 'PartyPrepSettingsRoundsPayload': return accessor(index, new PartyPrepSettingsRoundsPayload())! as PartyPrepSettingsRoundsPayload;
    case 'PartyPrepSettingsMiniGamePayload': return accessor(index, new PartyPrepSettingsMiniGamePayload())! as PartyPrepSettingsMiniGamePayload;
    default: return null;
  }
}
