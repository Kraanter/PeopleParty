import type { MiniGamePayloadType } from "@/flatbuffers/mini-game-payload-type"
import { HighwayHustleHostPayload } from "@/flatbuffers/highway-hustle-host-payload"
import type { HighwayHustleData, HighwayHustleEntity, HighwayHustlePlayerData } from "./HighwayHustleModels"
import { HighwayHustlePlayerPayload } from "@/flatbuffers/highway-hustle-player-payload"

export function parseHighwayHustleHostPayload(data: MiniGamePayloadType): HighwayHustleData {
    const payload: HighwayHustleHostPayload = data.gamestatepayload(new HighwayHustleHostPayload())

    const players: HighwayHustleEntity[] = []
    for (let i = 0; i < payload.entitiesLength(); i++) {
        players.push({x: payload.entities(i).xPos(), y: payload.entities(i).yPos()})
    }

    const obstacles: HighwayHustleEntity[] = []
    for (let i = 0; i < payload.obstaclesLength(); i++) {
        obstacles.push({x: payload.obstacles(i).xPos(), y: payload.obstacles(i).yPos()})
    }

    return {
        players: players,
        obstacles: obstacles,
        distance: Number(payload.distance())
    }
}

export function parseHighwayHustlePlayerPayload(data: MiniGamePayloadType): HighwayHustlePlayerData {
    const payload: HighwayHustlePlayerPayload = data.gamestatepayload(new HighwayHustlePlayerPayload())

    return {
        score: Number(payload.score())
    }
}