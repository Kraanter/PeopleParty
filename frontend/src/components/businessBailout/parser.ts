import { BusinessBailoutHostPayload } from '@/flatbuffers/business-bailout-host-payload'
import { BusinessBailoutPlayerPayload } from '@/flatbuffers/business-bailout-player-payload'
import { BusinessBailoutResultPayload } from '@/flatbuffers/business-bailout-result-payload'
import type { MiniGamePayloadType } from '@/flatbuffers/mini-game-payload-type'

export function parseBusinessBailoutPlayerPayload(payload: MiniGamePayloadType) {
  const bbpp: BusinessBailoutPlayerPayload = payload.gamestatepayload(
    new BusinessBailoutPlayerPayload()
  )
  const value = bbpp.value()
  const locked = bbpp.locked()

  return {
    value,
    locked: !!locked
  }
}

export type BailedPlayer = {
  name: string
  time: number
  value: number
}

export function parseBusinessBailoutHostPayload(payload: MiniGamePayloadType) {
  const bbhp: BusinessBailoutHostPayload = payload.gamestatepayload(
    new BusinessBailoutHostPayload()
  )
  const value = bbhp.value()
  const time = bbhp.time()
  const bailed_players: BailedPlayer[] = []
  for (let i = 0; i < bbhp.bailedPlayersLength(); i++) {
    const bailedPlayer = bbhp.bailedPlayers(i)
    const name = decodeURI(bailedPlayer?.name() ?? '')
    const time = bailedPlayer?.time()
    const value = bailedPlayer?.value()
    if (name && time && value) bailed_players.push({ name, time, value })
  }
  bailed_players.filter((bailedPlayer) => bailedPlayer.name && bailedPlayer.time)

  return {
    value,
    time,
    bailed_players
  }
}

export function parseBusinessBailoutResultPayload(payload: MiniGamePayloadType) {
  const bbhp: BusinessBailoutResultPayload = payload.gamestatepayload(
    new BusinessBailoutResultPayload()
  )

  const newSubmittedPlayers: BailedPlayer[] = []
  for (let i = 0; i < bbhp.resultsLength(); i++) {
    const submittedString = bbhp.results(i)
    if (submittedString === null) continue
    newSubmittedPlayers.push({
      name: decodeURI(submittedString.name() || ''),
      time: submittedString.time(),
      value: submittedString.value()
    })
  }

  return {
    submittedPlayers: newSubmittedPlayers
  }
}
