import type { MiniGamePayloadType } from '@/flatbuffers/mini-game-payload-type'
import type {
  UnscrambledHostData,
  UnscrambledPlayerData,
  UnscrambledResultPair,
  UnscrambledResults,
  UnscrambledRoundResultPair,
  UnscrambledRoundResults
} from './UnscrambledModels'
import { UnscrambledHostPayload } from '@/flatbuffers/unscrambled-host-payload'
import { UnscrambledPlayerPayload } from '@/flatbuffers/unscrambled-player-payload'
import { UnscrambledRoundResultPayload } from '@/flatbuffers/unscrambled-round-result-payload'
import { UnscrambledResultPayload } from '@/flatbuffers/unscrambled-result-payload'

export function parseUnscrambledHostPayload(data: MiniGamePayloadType): UnscrambledHostData {
  const payload: UnscrambledHostPayload = data.gamestatepayload(new UnscrambledHostPayload())

  return {
    time: Number(payload.time()),
    round: payload.round(),
    scrambled_word: payload.scrambledWord() || ''
  }
}

export function parseUnscrambledPlayerPayload(data: MiniGamePayloadType): UnscrambledPlayerData {
  const payload: UnscrambledPlayerPayload = data.gamestatepayload(new UnscrambledPlayerPayload())
  const words: string[] = []

  for (let i = 0; i < payload.wordsLength(); i++) {
    const word = payload.words(i)
    if (word) {
      words.push(word)
    }
  }

  return {
    time: Number(payload.time()),
    round: payload.round(),
    words: words,
    submitted: payload.submitted()
  }
}

export function parseUnscrambledRoundResults(
  data: MiniGamePayloadType,
  clientName: string = ''
): UnscrambledRoundResults {
  const miniGameResultPayload: UnscrambledRoundResultPayload = data.gamestatepayload(
    new UnscrambledRoundResultPayload()
  )

  const results: UnscrambledRoundResultPair[] = []
  for (let i = 0; i < miniGameResultPayload.resultsLength(); i++) {
    const result = miniGameResultPayload.results(i)
    if (clientName == '' || result.name() == clientName) {
      results.push({
        name: decodeURI(result.name()) || '',
        submitted: result.submitted(),
        guess: result.guess(),
        time_taken: Number(result.timeTaken())
      })
    }
  }

  return {
    round: miniGameResultPayload.round(),
    correct: miniGameResultPayload.correct(),
    results: results
  }
}

export function parseUnscrambledResults(data: MiniGamePayloadType): UnscrambledResults {
  const miniGameResultPayload: UnscrambledResultPayload = data.gamestatepayload(
    new UnscrambledResultPayload()
  )

  const results: UnscrambledResultPair[] = []
  for (let i = 0; i < miniGameResultPayload.minigameResultsLength(); i++) {
    const result = miniGameResultPayload.minigameResults(i)

    results.push({
      name: decodeURI(result.name()) || '',
      correct: result.correct(),
      time_taken: Number(result.timeTaken()),
      placement: result.placement()
    })
  }

  return {
    results: results
  }
}
