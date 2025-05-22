import type { MiniGamePayloadType } from "@/flatbuffers/mini-game-payload-type"
import { HighwayHustleHostPayload } from "@/flatbuffers/highway-hustle-host-payload"
import type { HighwayHustleData, HighwayHustleEntity, HighwayHustlePlayerData, HighwayHustleResult, HighwayHustleResultPair } from "./HighwayHustleModels"
import { HighwayHustlePlayerPayload } from "@/flatbuffers/highway-hustle-player-payload"
import { HighwayHustleResultPayload } from "@/flatbuffers/highway-hustle-result-payload"

export function parseHighwayHustleHostPayload(data: MiniGamePayloadType): HighwayHustleData {
    const payload: HighwayHustleHostPayload = data.gamestatepayload(new HighwayHustleHostPayload())

    const players: HighwayHustleEntity[] = []
    for (let i = 0; i < payload.entitiesLength(); i++) {
        players.push({
            id: payload.entities(i).id(), 
            x: payload.entities(i).xPos(), 
            y: payload.entities(i).yPos(),
            carType: payload.entities(i).carType()
        })
    }

    const obstacles: HighwayHustleEntity[] = []
    for (let i = 0; i < payload.obstaclesLength(); i++) {
        obstacles.push({
            id: payload.obstacles(i).id(), 
            x: payload.obstacles(i).xPos(), 
            y: payload.obstacles(i).yPos(),
            carType: payload.obstacles(i).carType()
        })
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
        score: Number(payload.score()),
        isDead: payload.isDead(),
        carType: payload.carType()
    }
}

export function parseHighwayHustleResultPayload(data: MiniGamePayloadType): HighwayHustleResult {
    const payload: HighwayHustleResultPayload = data.gamestatepayload(new HighwayHustleResultPayload())

    const results: HighwayHustleResultPair[] = []
    for (let i = 0; i < payload.resultsLength(); i++) {
        results.push({
            name: payload.results(i).name(),
            score: Number(payload.results(i).score()),
            placement: Number(payload.results(i).placement())
        })
    }

    results.sort((a, b) => a.placement - b.placement)

    return {
        results: results
    }
}