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
  carType: number;
}

// player
export interface HighwayHustlePlayerData {
  score: number;
  isDead: boolean;
  carType: number;
}

// result
export interface HighwayHustleResultPair {
  name: string
  score: number
  placement: number
}

export interface HighwayHustleResult {
  results: HighwayHustleResultPair[]
}