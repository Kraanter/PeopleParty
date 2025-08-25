// host
export interface MMVec2 { x: number; y: number }

export type MMShape =
  | { kind: 'circle'; radius: number }
  | { kind: 'rect'; width: number; height: number }
  | { kind: 'poly'; vertices: MMVec2[] } // local-space verts, rotation applied at entity

export interface MarbleManiaEntity {
  id: string;
  type: 'marble' | 'obstacle';
  pos: MMVec2;            // CENTER position in world coords
  rotation: number;       // radians
  finished: boolean;      // marbles only
  player_name?: string;   // marbles only
  shape: MMShape;
  restitution?: number;
  friction?: number;
}

export interface MarbleManiaData {
  entities: MarbleManiaEntity[];
  game_phase: number; // 0 = placement, 1 = simulation, 2 = finished
  placement_time_left: number;
  finish_line_y: number;
  world_min: MMVec2;
  world_max: MMVec2;
}

// player
export interface MarbleManiaPlayerData {
  id: string;
  game_phase: number; // 0 = placement, 1 = simulation, 2 = finished
  placement_time_left: number;
  y_pos: number;
  finish_line_y: number;
}

// result
export interface MarbleManiaResultPair {
  name: string;
  placement: number;
  time_to_finish: number;
  has_finished: boolean;
}

export interface MarbleManiaResult {
  results: MarbleManiaResultPair[]
}
