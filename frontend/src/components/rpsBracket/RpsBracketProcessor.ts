import type { MiniGamePayloadType } from '@/flatbuffers/messageClass'
import { RPSBracketHostPayload } from '@/flatbuffers/messageClass'
import type { RpsBracketHostData } from './RpsBracketModels'

export function parseRpsBracketHostPayload(data: MiniGamePayloadType): RpsBracketHostData {
  const payload: RPSBracketHostPayload = data.gamestatepayload(new RPSBracketHostPayload())

  const matches = []
  for (let i = 0; i < payload.matchesLength(); i++) {
    const match = payload.matches(i)
    matches.push({
      left: { name: decodeURI(match?.player1() || '') },
      right: { name: decodeURI(match?.player2() || '') },
      winner: { name: decodeURI(match?.winner() || '') }
    })
  }

  return { matches }
}
