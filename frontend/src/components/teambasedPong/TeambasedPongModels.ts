export interface TeamPlayer {
  name: string
}

export interface TeambasedPongHostData {
  current_round: number
  time_left: number
  map_width: number
  map_height: number
  ball_x: number
  ball_y: number
  paddle_a_x: number
  paddle_a_y: number
  paddle_b_x: number
  paddle_b_y: number
  paddle_width: number
  paddle_height: number
  team_a_players: TeamPlayer[]
  team_b_players: TeamPlayer[]
}

export enum PongTeam {
  TEAM_A = 0,
  TEAM_B = 1,
  SPECTATOR = 2
}

export interface TeambasedPongPlayerData {
  current_round: number
  time_left: number
  your_team: PongTeam
  is_still_playing: boolean
}

export enum PongRoundWinner {
  TEAM_A_WON = 0,
  TEAM_B_WON = 1,
  TIE = 2,
  SINGLE_WINNER = 3
}

export interface NextTeamPlayer {
  name: string
}

export interface TeambasedPongRoundResult {
  round_winner: PongRoundWinner
  winning_player_name: string
  time_left: number
  has_next_round: boolean
  next_team_a_players: NextTeamPlayer[]
  next_team_b_players: NextTeamPlayer[]
}

export interface TeambasedPongResultPair {
  name: string
  placement: number
  rounds_won: number
}

export interface TeambasedPongResult {
  results: TeambasedPongResultPair[]
}
