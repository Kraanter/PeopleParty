import type { MemoryMixerGridCell, MemoryMixerGridPayload, MemoryMixerResultPayload, MemoryMixerRoundResultPayload } from "@/flatbuffers/messageClass"

enum MemoryMixerIcon {
    Balloon = "assets/games/memoryMixer/balloon.svg",
    Cake = "assets/games/memoryMixer/birthdaycake.svg",
    Confetti = "assets/games/memoryMixer/confettiball.svg",
    Popper = "assets/games/memoryMixer/partypopper.svg",
    Face = "assets/games/memoryMixer/partyface.svg",
    //Empty = "",
};

interface MemoryMixerCell {
    icon: MemoryMixerIcon
    players_on_card: number
    is_highlighted: boolean
}

export interface MemoryMixerGrid {
    timeLeft: number
    maxOnCard: number
    phase: number
    round: number
    active_players: number
    submittedNames: string[]
    grid: MemoryMixerCell[][]
}

export interface PlayerSubmittedData {
    playerSubmitted: boolean,
    x: number,
    y: number
}

export interface RoundResult {
    round: number,
    correctNames: string[]
    wrongNames: string[]
}

interface ResultsPair {
    placement: number;
    name: string;
    rounds_won: number;
}

export interface MiniGameResult {
    round: number;
    results: ResultsPair[];
}

export const processRoundResult = (payload: MemoryMixerRoundResultPayload): RoundResult => {
    const correctNames: string[] = []
    for (let i = 0; i < payload.correctNamesLength(); i++) {
        correctNames.push(decodeURI(payload.correctNames(i)!))
    }

    const wrongNames: string[] = []
    for (let i = 0; i < payload.wrongNamesLength(); i++) {
        wrongNames.push(decodeURI(payload.wrongNames(i)!))
    }

    return {
        round: payload.round(),
        correctNames,
        wrongNames
    }
}

export const processMiniGameResult = (payload: MemoryMixerResultPayload): MiniGameResult => {
    const results: ResultsPair[] = []
    for (let i = 0; i < payload.minigameResultsLength(); i++) {
        const result = payload.minigameResults(i)
        results.push({
            placement: result?.placement() || 0,
            name: decodeURI(result?.name()) || "",
            rounds_won: result?.roundsWon() || 0
        })
    }
    return {
        round: payload.round(),
        results
    }
}

export const processGrid = (payload: MemoryMixerGridPayload): MemoryMixerGrid => {
    const grid: MemoryMixerCell[][] = []
    for (let i = 0; i < payload.gridLength(); i++) {
        const row: MemoryMixerCell[] = []
        for (let j = 0; j < payload.grid(i)?.rowLength()!; j++) {
            const cell = payload.grid(i)?.row(j)
            row.push({
                icon: Object.values(MemoryMixerIcon)[cell?.icon() || 0],
                players_on_card: cell?.playersOnCard() || 0,
                is_highlighted: cell?.isHighlighted() || false
            })
        }
        grid.push(row)
    }

    const names: string[] = []
    for (let i = 0; i < payload.namesLength(); i++) {
        names.push(decodeURI(payload.names(i)!))
    }
    return {
        timeLeft: Number(payload.timeLeft()),
        maxOnCard: payload.maxOnCard(),
        phase: payload.phase(),
        round: payload.round(),
        active_players: payload.playersLeft(),
        submittedNames: names,
        grid
    }
}