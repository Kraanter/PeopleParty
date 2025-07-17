import type { MiniGamePayloadType } from "@/flatbuffers/mini-game-payload-type"
import { MarbleManiaHostPayload } from "@/flatbuffers/marble-mania-host-payload"
import type { MarbleManiaData, MarbleManiaEntity, MarbleManiaPlayerData, MarbleManiaResult, MarbleManiaResultPair } from "./MarbleManiaModels"
import { MarbleManiaResultPayload } from "@/flatbuffers/marble-mania-result-payload"
import { MarbleManiaPlayerInputPayload } from "@/flatbuffers/marble-mania-player-input-payload"

export function parseMarbleManiaHostPayload(data: MiniGamePayloadType): MarbleManiaData {
    const payload: MarbleManiaHostPayload = data.gamestatepayload(new MarbleManiaHostPayload())

    const entities: MarbleManiaEntity[] = []
    for (let i = 0; i < payload.entitiesLength(); i++) {
        entities.push({
            id: payload.entities(i).id(),
            x_pos: payload.entities(i).xPos(),
            y_pos: payload.entities(i).yPos(),
            entity_type: payload.entities(i).entityType(),
            is_finished: payload.entities(i).isFinished()
        })
    }

    return {
        entities: entities,
        game_phase: payload.gamePhase(),
        placement_time_left: payload.placementTimeLeft(),
        finish_line_y: payload.finishLineY()
    }
}

export function parseMarbleManiaPlayerInputPayload(data: MiniGamePayloadType): MarbleManiaPlayerData {
    const payload: MarbleManiaPlayerInputPayload = data.gamestatepayload(new MarbleManiaPlayerInputPayload())

    return {
        x_pos: payload.xPos(),
        y_pos: payload.yPos(),
        action_type: payload.actionType() // 0 = place_marble, 1 = ready_up
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