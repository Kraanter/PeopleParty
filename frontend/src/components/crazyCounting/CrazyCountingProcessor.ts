import type { MiniGamePayloadType } from '@/flatbuffers/messageClass'
import {
  CrazyCountingHostEntitiesPayload,
  CrazyCountingResultPayload,
  FBCrazyCountingEntity,
  FBCrazyCountingResultPair
} from '@/flatbuffers/messageClass'
import type { CrazyCountingHostData, CrazyCountingResult } from './CrazyCountingModels'

export function parseCrazyCountingHostPayload(data: MiniGamePayloadType): CrazyCountingHostData {
  const payload: CrazyCountingHostEntitiesPayload = data.gamestatepayload(
    new CrazyCountingHostEntitiesPayload()
  )

  const entities = []
  for (let i = 0; i < payload.entitiesLength(); i++) {
    const entity: FBCrazyCountingEntity | null = payload.entities(i, new FBCrazyCountingEntity())
    if (entity === null) continue
    entities.push({ x_pos: entity.xPos(), y_pos: entity.yPos() })
  }

  const submitted = []
  for (let i = 0; i < payload.submittedLength(); i++) {
    const name = payload.submitted(i)
    if (name === null) continue
    submitted.push(decodeURI(name))
  }

  return {
    entities,
    time_left: Number(payload.timeLeft()),
    submitted
  }
}

export function parseCrazyCountingResultPayload(data: MiniGamePayloadType): CrazyCountingResult {
  const payload: CrazyCountingResultPayload = data.gamestatepayload(
    new CrazyCountingResultPayload()
  )

  const results = []
  for (let i = 0; i < payload.resultsLength(); i++) {
    const pair: FBCrazyCountingResultPair | null = payload.results(
      i,
      new FBCrazyCountingResultPair()
    )
    if (pair === null) continue
    results.push({ name: decodeURI(pair.name() || ''), guess: pair.guess() })
  }

  return {
    correct_answer: payload.correctAnswer(),
    results
  }
}
