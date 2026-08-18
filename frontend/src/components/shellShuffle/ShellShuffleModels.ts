export enum ShellShufflePhase {
  REVEAL = 0,
  SHUFFLE = 1,
  GUESS = 2,
  ROUND_RESULT = 3
}

export interface ShellShuffleCup {
  x_pos: number
  depth: number  // sin-arc value: +1 = closest, -1 = farthest, 0 = idle
}

export interface ShellShuffleHostData {
  phase: ShellShufflePhase
  current_round: number
  time_left: number       // -1 during SHUFFLE (no countdown)
  cups: ShellShuffleCup[]
  ball_cup_index: number  // array index of ball cup; -1 if hidden
  active_players: number
  map_width: number
}

export interface ShellShufflePlayerData {
  phase: number           // 0-3, use ShellShufflePhase for comparison
  current_round: number
  time_left: number
  num_cups: number
  is_eliminated: boolean
  has_guessed: boolean
  was_correct: boolean
}

export interface ShellShuffleRoundPlayerResult {
  name: string
  was_correct: boolean
  guessed_cup: number     // -1 = didn't guess in time
}

export interface ShellShuffleRoundResult {
  current_round: number
  correct_cup_index: number  // left-to-right 0-based position
  player_results: ShellShuffleRoundPlayerResult[]
  players_remaining: number
}

export interface ShellShuffleResultPair {
  name: string
  placement: number
  rounds_survived: number
}

export interface ShellShuffleResult {
  results: ShellShuffleResultPair[]
}
