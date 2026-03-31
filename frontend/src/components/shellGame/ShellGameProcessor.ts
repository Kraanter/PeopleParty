import type { MiniGamePayloadType } from '@/flatbuffers/messageClass'
import {
  FBShellGameCup,
  FBShellGameResultPair,
  FBShellGameRoundPlayerResult,
  ShellGameHostPayload,
  ShellGamePlayerPayload,
  ShellGameResultPayload,
  ShellGameRoundResultPayload
} from '@/flatbuffers/messageClass'
import type {
  ShellGameHostData,
  ShellGamePlayerData,
  ShellGameResult,
  ShellGameRoundResult
} from './ShellGameModels'
import { ShellGamePhase } from './ShellGameModels'

export function parseShellGameHostPayload(data: MiniGamePayloadType): ShellGameHostData {
  const payload: ShellGameHostPayload = data.gamestatepayload(new ShellGameHostPayload())

  const cups = []
  for (let i = 0; i < payload.cupsLength(); i++) {
    const cup: FBShellGameCup | null = payload.cups(i, new FBShellGameCup())
    if (cup) {
      cups.push({ x_pos: cup.xPos() })
    }
  }

  return {
    phase: payload.phase() as ShellGamePhase,
    current_round: payload.currentRound(),
    time_left: payload.timeLeft(),
    cups,
    ball_cup_index: payload.ballCupIndex(),
    active_players: payload.activePlayers(),
    map_width: payload.mapWidth()
  }
}

export function parseShellGamePlayerPayload(data: MiniGamePayloadType): ShellGamePlayerData {
  const payload: ShellGamePlayerPayload = data.gamestatepayload(new ShellGamePlayerPayload())

  return {
    phase: payload.phase(),
    current_round: payload.currentRound(),
    time_left: payload.timeLeft(),
    num_cups: payload.numCups(),
    is_eliminated: payload.isEliminated(),
    has_guessed: payload.hasGuessed(),
    was_correct: payload.wasCorrect()
  }
}

export function parseShellGameRoundResult(data: MiniGamePayloadType): ShellGameRoundResult {
  const payload: ShellGameRoundResultPayload = data.gamestatepayload(
    new ShellGameRoundResultPayload()
  )

  const player_results = []
  for (let i = 0; i < payload.playerResultsLength(); i++) {
    const r: FBShellGameRoundPlayerResult | null = payload.playerResults(
      i,
      new FBShellGameRoundPlayerResult()
    )
    if (r) {
      player_results.push({
        name: decodeURI(r.name() || ''),
        was_correct: r.wasCorrect(),
        guessed_cup: r.guessedCup()
      })
    }
  }

  return {
    current_round: payload.currentRound(),
    correct_cup_index: payload.correctCupIndex(),
    player_results,
    players_remaining: payload.playersRemaining()
  }
}

export function parseShellGameResult(data: MiniGamePayloadType): ShellGameResult {
  const payload: ShellGameResultPayload = data.gamestatepayload(new ShellGameResultPayload())

  const results = []
  for (let i = 0; i < payload.resultsLength(); i++) {
    const r: FBShellGameResultPair | null = payload.results(i, new FBShellGameResultPair())
    if (r) {
      results.push({
        name: decodeURI(r.name() || ''),
        placement: r.placement(),
        rounds_survived: r.roundsSurvived()
      })
    }
  }

  return { results }
}
