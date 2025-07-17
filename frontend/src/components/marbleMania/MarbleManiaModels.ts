// host
export interface MarbleManiaData {
  entities: MarbleManiaEntity[];
  game_phase: number; // 0 = placement, 1 = simulation, 2 = finished
  placement_time_left: number;
  finish_line_y: number;
}

export interface MarbleManiaEntity {
  id: string;
  x_pos: number;
  y_pos: number;
  entity_type: number; // 0 = marble, 1 = obstacle
  is_finished: boolean;
}

// player
export interface MarbleManiaPlayerData {
  x_pos: number;
  y_pos: number;
  action_type: number; // 0 = place_marble, 1 = ready_up
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