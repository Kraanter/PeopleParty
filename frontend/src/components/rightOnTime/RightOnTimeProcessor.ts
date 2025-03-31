import type { MiniGamePayloadType } from "@/flatbuffers/mini-game-payload-type"
import { RightOnTimePayload } from "@/flatbuffers/right-on-time-payload"
import { RightOnTimeResultPayload } from "@/flatbuffers/right-on-time-result-payload"
import type { RightOnTimeData, RightOnTimeResultPair, RightOnTimeResults, RightOnTimeRoundResultPair, RightOnTimeRoundResults } from "./RightOnTimeModels"
import { RightOnTimeRoundResultPayload } from "@/flatbuffers/right-on-time-round-result-payload"


export function parseRightOnTimePayload(data: MiniGamePayloadType): RightOnTimeData {
    const payload: RightOnTimePayload = data.gamestatepayload(new RightOnTimePayload())

    const submitted: string[] = []
    for (let i = 0; i < payload.submittedLength(); i++) {
        submitted.push(payload.submitted(i))
    }

    return {
        round: payload.round(),
        target: payload.target(),
        time: Number(payload.time()),
        fade_out: payload.fadeOut(),
        submitted: submitted
    }
}

export function parseRightOnTimeRoundResults(data: MiniGamePayloadType, clientName: string = ''): RightOnTimeRoundResults {
    const miniGameResultPayload: RightOnTimeRoundResultPayload = data.gamestatepayload(new RightOnTimeRoundResultPayload())

    const results: RightOnTimeRoundResultPair[] = []
    for (let i = 0; i < miniGameResultPayload.resultsLength(); i++) {
        const result = miniGameResultPayload.results(i)
        if (clientName == '' || result.name() == clientName) {
            results.push({
            player: decodeURI(result.name()) || '',
            diff: Number(result.diff()),
            })
        }
    }

    return {
        results: results.sort((a, b) => Math.abs(a.diff) - Math.abs(b.diff)),
        round: miniGameResultPayload.round(),
        target: miniGameResultPayload.target()
    }
}

export function parseRightOnTimeResults(data: MiniGamePayloadType): RightOnTimeResults {
    const miniGameResultPayload: RightOnTimeResultPayload = data.gamestatepayload(new RightOnTimeResultPayload())

    const results: RightOnTimeResultPair[] = []
    for (let i = 0; i < miniGameResultPayload.minigameResultsLength(); i++) {
        const result = miniGameResultPayload.minigameResults(i)

        results.push({
        name: decodeURI(result.name()) || '',
        round_one_diff: Number(result.roundOneDiff()),
        round_two_diff: Number(result.roundTwoDiff()),
        round_three_diff: Number(result.roundThreeDiff()),
        average_diff_time: Number(result.averageDiffTime())
        })
    }

    return {
    results: results
    }
}