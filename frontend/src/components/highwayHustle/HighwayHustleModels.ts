// host
export interface HighwayHustleData {
  players: HighwayHustleEntity[]
  obstacles: HighwayHustleEntity[]
  distance: number
}

export interface HighwayHustleEntity {
  id: string
  x: number
  y: number
  carType: number
  isDead: boolean
}

// player
export interface HighwayHustlePlayerData {
  score: number
  isDead: boolean
  carType: number
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

// canvas snapshot — combines whichever of race/results is currently live so the GameView can
// stay a single canvas (matching the pre-migration single-Application design) without needing
// two separately-watched props racing each other
export type HighwayHustlePhase = 'race' | 'results'

export interface HighwayHustleSnapshot {
  phase: HighwayHustlePhase
  payload: HighwayHustleData
  results: HighwayHustleResult
}
