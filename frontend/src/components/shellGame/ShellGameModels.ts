export enum ShellGamePhase {
  REVEAL = 0,
  SHUFFLE = 1,
  GUESS = 2,
  ROUND_RESULT = 3
}

export interface ShellGameCup {
  x_pos: number
  depth: number  // sin-arc value: +1 = closest, -1 = farthest, 0 = idle
}

export interface ShellGameHostData {
  phase: ShellGamePhase
  current_round: number
  time_left: number       // -1 during SHUFFLE (no countdown)
  cups: ShellGameCup[]
  ball_cup_index: number  // array index of ball cup; -1 if hidden
  active_players: number
  map_width: number
}

export interface ShellGamePlayerData {
  phase: number           // 0-3, use ShellGamePhase for comparison
  current_round: number
  time_left: number
  num_cups: number
  is_eliminated: boolean
  has_guessed: boolean
  was_correct: boolean
}

export interface ShellGameRoundPlayerResult {
  name: string
  was_correct: boolean
  guessed_cup: number     // -1 = didn't guess in time
}

export interface ShellGameRoundResult {
  current_round: number
  correct_cup_index: number  // left-to-right 0-based position
  player_results: ShellGameRoundPlayerResult[]
  players_remaining: number
}

export interface ShellGameResultPair {
  name: string
  placement: number
  rounds_survived: number
}

export interface ShellGameResult {
  results: ShellGameResultPair[]
}
