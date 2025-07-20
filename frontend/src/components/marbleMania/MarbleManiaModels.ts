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
  player_name?: string; // Player name for marbles (empty for obstacles)
  // Obstacle-specific properties (ignored for marbles)
  obstacle_type?: number; // 0 = static_circle, 1 = static_rectangle, 2 = moving_circle, 3 = moving_rectangle, 4 = spinning_circle, 5 = spinning_rectangle
  is_circle?: boolean;
  width?: number;
  height?: number;
  rotation?: number; // Current rotation angle in radians
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