import type { MiniGamePayloadType } from "@/flatbuffers/mini-game-payload-type"
import { MarbleManiaHostPayload } from "@/flatbuffers/marble-mania-host-payload"
import type { MarbleManiaData, MarbleManiaEntity, MarbleManiaPlayerData, MarbleManiaResult, MarbleManiaResultPair } from "./MarbleManiaModels"
import { MarbleManiaResultPayload } from "@/flatbuffers/marble-mania-result-payload"
import { MarbleManiaPlayerPayload } from "@/flatbuffers/marble-mania-player-payload"

export function parseMarbleManiaHostPayload(data: MiniGamePayloadType): MarbleManiaData {
    const payload: MarbleManiaHostPayload = data.gamestatepayload(new MarbleManiaHostPayload())

    const entities: MarbleManiaEntity[] = []
    for (let i = 0; i < payload.entitiesLength(); i++) {
        const entity = payload.entities(i)
        entities.push({
            id: entity.id(),
            x_pos: entity.xPos(),
            y_pos: entity.yPos(),
            entity_type: entity.entityType(),
            is_finished: entity.isFinished(),
            player_name: entity.playerName() ? decodeURI(entity.playerName()) : undefined,
            obstacle_type: entity.obstacleType(),
            is_circle: entity.isCircle(),
            width: entity.width(),
            height: entity.height(),
            rotation: entity.rotation()
        })
    }

    return {
        entities: entities,
        game_phase: payload.gamePhase(),
        placement_time_left: payload.placementTimeLeft(),
        finish_line_y: payload.finishLineY()
    }
}

export function parseMarbleManiaPlayerPayload(data: MiniGamePayloadType): MarbleManiaPlayerData {
    const payload: MarbleManiaPlayerPayload = data.gamestatepayload(new MarbleManiaPlayerPayload())

    return {
        id: payload.id(),
        game_phase: payload.gamePhase(),
        placement_time_left: payload.placementTimeLeft(),
        y_pos: payload.yPos(),
        finish_line_y: payload.finishLineY()
    }
}

export function parseMarbleManiaResultPayload(data: MiniGamePayloadType): MarbleManiaResult {
    const payload: MarbleManiaResultPayload = data.gamestatepayload(new MarbleManiaResultPayload())

    const results: MarbleManiaResultPair[] = []
    for (let i = 0; i < payload.resultsLength(); i++) {
        results.push({
            name: payload.results(i).name(),
            placement: payload.results(i).placement(),
            time_to_finish: payload.results(i).timeToFinish(),
            has_finished: payload.results(i).hasFinished()
        })
    }

    results.sort((a, b) => a.placement - b.placement)

    return {
        results: results
    }
}