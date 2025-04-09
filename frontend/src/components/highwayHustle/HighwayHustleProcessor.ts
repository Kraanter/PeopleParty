import type { MiniGamePayloadType } from "@/flatbuffers/mini-game-payload-type"
import { HighwayHustleHostPayload } from "@/flatbuffers/highway-hustle-host-payload"
import type { HighwayHustleData, HighwayHustlePlayer, HighwayHustlePlayerData } from "./HighwayHustleModels"
import { HighwayHustlePlayerPayload } from "@/flatbuffers/highway-hustle-player-payload"

export function parseHighwayHustleHostPayload(data: MiniGamePayloadType): HighwayHustleData {
    const payload: HighwayHustleHostPayload = data.gamestatepayload(new HighwayHustleHostPayload())

    const players: HighwayHustlePlayer[] = []
    for (let i = 0; i < payload.entitiesLength(); i++) {
        players.push({x: payload.entities(i).xPos(), y: payload.entities(i).yPos()})
    }
    return {
        players: players
    }
}

export function parseHighwayHustlePlayerPayload(data: MiniGamePayloadType): HighwayHustlePlayerData {
    const payload: HighwayHustlePlayerPayload = data.gamestatepayload(new HighwayHustlePlayerPayload())

    return {
        score: Number(payload.score())
    }
}