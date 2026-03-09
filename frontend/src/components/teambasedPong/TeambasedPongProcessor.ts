import type { MiniGamePayloadType } from '@/flatbuffers/messageClass'
import type {
  TeambasedPongHostData,
  TeambasedPongPlayerData,
  TeambasedPongRoundResult,
  TeambasedPongResult,
  TeamPlayer,
  NextTeamPlayer,
  TeambasedPongResultPair
} from './TeambasedPongModels'
import { PongTeam, PongRoundWinner } from './TeambasedPongModels'
import { TeambasedPongHostPayload } from '@/flatbuffers/teambased-pong-host-payload'
import { TeambasedPongPlayerPayload } from '@/flatbuffers/teambased-pong-player-payload'
import { TeambasedPongRoundResultPayload } from '@/flatbuffers/teambased-pong-round-result-payload'
import { TeambasedPongResultPayload } from '@/flatbuffers/teambased-pong-result-payload'

export function parseTeambasedPongHostPayload(data: MiniGamePayloadType): TeambasedPongHostData {
  const payload: TeambasedPongHostPayload = data.gamestatepayload(
    new TeambasedPongHostPayload()
  )

  const team_a_players: TeamPlayer[] = []
  for (let i = 0; i < payload.teamAPlayersLength(); i++) {
    const player = payload.teamAPlayers(i)
    if (player) {
      team_a_players.push({
        name: player.name() || ''
      })
    }
  }

  const team_b_players: TeamPlayer[] = []
  for (let i = 0; i < payload.teamBPlayersLength(); i++) {
    const player = payload.teamBPlayers(i)
    if (player) {
      team_b_players.push({
        name: player.name() || ''
      })
    }
  }

  return {
    current_round: payload.currentRound(),
    time_left: payload.timeLeft(),
    map_width: payload.mapWidth(),
    map_height: payload.mapHeight(),
    ball_x: payload.ballX(),
    ball_y: payload.ballY(),
    paddle_a_x: payload.paddleAX(),
    paddle_a_y: payload.paddleAY(),
    paddle_b_x: payload.paddleBX(),
    paddle_b_y: payload.paddleBY(),
    paddle_width: payload.paddleWidth(),
    paddle_height: payload.paddleHeight(),
    team_a_players: team_a_players,
    team_b_players: team_b_players
  }
}

export function parseTeambasedPongPlayerPayload(
  data: MiniGamePayloadType
): TeambasedPongPlayerData {
  const payload: TeambasedPongPlayerPayload = data.gamestatepayload(
    new TeambasedPongPlayerPayload()
  )

  return {
    current_round: payload.currentRound(),
    time_left: payload.timeLeft(),
    your_team: payload.yourTeam() as PongTeam,
    is_still_playing: payload.isStillPlaying()
  }
}

export function parseTeambasedPongRoundResult(
  data: MiniGamePayloadType
): TeambasedPongRoundResult {
  const payload: TeambasedPongRoundResultPayload = data.gamestatepayload(
    new TeambasedPongRoundResultPayload()
  )

  const next_team_a_players: NextTeamPlayer[] = []
  for (let i = 0; i < payload.nextTeamAPlayersLength(); i++) {
    const player = payload.nextTeamAPlayers(i)
    if (player) {
      next_team_a_players.push({
        name: player.name() || ''
      })
    }
  }

  const next_team_b_players: NextTeamPlayer[] = []
  for (let i = 0; i < payload.nextTeamBPlayersLength(); i++) {
    const player = payload.nextTeamBPlayers(i)
    if (player) {
      next_team_b_players.push({
        name: player.name() || ''
      })
    }
  }

  return {
    round_winner: payload.roundWinner() as PongRoundWinner,
    winning_player_name: payload.winningPlayerName() || '',
    time_left: payload.timeLeft(),
    is_first_round: payload.isFirstRound(),
    has_next_round: payload.hasNextRound(),
    next_team_a_players: next_team_a_players,
    next_team_b_players: next_team_b_players
  }
}

export function parseTeambasedPongResult(data: MiniGamePayloadType): TeambasedPongResult {
  const payload: TeambasedPongResultPayload = data.gamestatepayload(
    new TeambasedPongResultPayload()
  )

  const results: TeambasedPongResultPair[] = []
  for (let i = 0; i < payload.resultsLength(); i++) {
    const result = payload.results(i)
    if (result) {
      results.push({
        name: decodeURI(result.name()) || '',
        placement: result.placement(),
        rounds_won: result.roundsWon()
      })
    }
  }

  return {
    results: results
  }
}
