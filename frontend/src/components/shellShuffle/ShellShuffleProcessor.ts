import type { MiniGamePayloadType } from '@/flatbuffers/messageClass'
import {
  FBShellShuffleCup,
  FBShellShuffleResultPair,
  FBShellShuffleRoundPlayerResult,
  ShellShuffleHostPayload,
  ShellShufflePlayerPayload,
  ShellShuffleResultPayload,
  ShellShuffleRoundResultPayload
} from '@/flatbuffers/messageClass'
import type {
  ShellShuffleHostData,
  ShellShufflePlayerData,
  ShellShuffleResult,
  ShellShuffleRoundResult
} from './ShellShuffleModels'
import { ShellShufflePhase } from './ShellShuffleModels'

export function parseShellShuffleHostPayload(data: MiniGamePayloadType): ShellShuffleHostData {
  const payload: ShellShuffleHostPayload = data.gamestatepayload(new ShellShuffleHostPayload())

  const cups = []
  for (let i = 0; i < payload.cupsLength(); i++) {
    const cup: FBShellShuffleCup | null = payload.cups(i, new FBShellShuffleCup())
    if (cup) {
      cups.push({ x_pos: cup.xPos(), depth: cup.depth() })
    }
  }

  return {
    phase: payload.phase() as ShellShufflePhase,
    current_round: payload.currentRound(),
    time_left: payload.timeLeft(),
    cups,
    ball_cup_index: payload.ballCupIndex(),
    active_players: payload.activePlayers(),
    map_width: payload.mapWidth()
  }
}

export function parseShellShufflePlayerPayload(data: MiniGamePayloadType): ShellShufflePlayerData {
  const payload: ShellShufflePlayerPayload = data.gamestatepayload(new ShellShufflePlayerPayload())

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

export function parseShellShuffleRoundResult(data: MiniGamePayloadType): ShellShuffleRoundResult {
  const payload: ShellShuffleRoundResultPayload = data.gamestatepayload(
    new ShellShuffleRoundResultPayload()
  )

  const player_results = []
  for (let i = 0; i < payload.playerResultsLength(); i++) {
    const r: FBShellShuffleRoundPlayerResult | null = payload.playerResults(
      i,
      new FBShellShuffleRoundPlayerResult()
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

export function parseShellShuffleResult(data: MiniGamePayloadType): ShellShuffleResult {
  const payload: ShellShuffleResultPayload = data.gamestatepayload(new ShellShuffleResultPayload())

  const results = []
  for (let i = 0; i < payload.resultsLength(); i++) {
    const r: FBShellShuffleResultPair | null = payload.results(i, new FBShellShuffleResultPair())
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
