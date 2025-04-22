// host
export interface HighwayHustleData {
  players: HighwayHustleEntity[];
  obstacles: HighwayHustleEntity[];
  distance: number;
}

export interface HighwayHustleEntity {
  id: string;
  x: number;
  y: number;
}

// player
export interface HighwayHustlePlayerData {
  score: number;
}

// result
