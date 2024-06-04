import type { MemoryMixerGridPayload } from "@/flatbuffers/messageClass"

enum MemoryMixerIcon {
    Balloon,
    Cake,
    Confetti,
    Confetti2,
    Smiley,
    Empty,
};

interface MemoryMixerCell {
    icon: MemoryMixerIcon
    players_on_card: number
}

export interface MemoryMixerGrid {
    timeLeft: number
    maxOnCard: number
    grid: MemoryMixerCell[][]
}

export interface PlayerSubmittedData {
    playerSubmitted: boolean,
    x: number,
    y: number
}

export const processGrid = (payload: MemoryMixerGridPayload): MemoryMixerGrid => {
    const grid: MemoryMixerCell[][] = []
    for (let i = 0; i < payload.gridLength(); i++) {
        const row: MemoryMixerCell[] = []
        for (let j = 0; j < payload.grid(i)?.rowLength()!; j++) {
            const cell = payload.grid(i)?.row(j)
            row.push({
                icon: MemoryMixerIcon.Empty, // fixme: actually get the icon
                players_on_card: cell?.playersOnCard() || 0
            })
        }
        grid.push(row)
    }
    return {
        timeLeft: Number(payload.timeLeft()),
        maxOnCard: Number(payload.maxOnCard()),
        grid
    }
}