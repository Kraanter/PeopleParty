// host
export interface HighwayHustleData {
  players: HighwayHustleEntity[];
  obstacles: HighwayHustleEntity[];
  distance: number;
}

export interface HighwayHustleEntity {
  x: number;
  y: number;
}

// player
export interface HighwayHustlePlayerData {
  score: number;
}

// result
