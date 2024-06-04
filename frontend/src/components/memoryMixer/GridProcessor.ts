import type { MemoryMixerGridCell, MemoryMixerGridPayload } from "@/flatbuffers/messageClass"

enum MemoryMixerIcon {
    Balloon = "🎈",
    Cake = "🎂",
    Confetti = "🎊",
    Popper = "🎉",
    Face = "🥳",
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
    grid: MemoryMixerCell[][]
}

export interface PlayerSubmittedData {
    playerSubmitted: boolean,
    x: number,
    y: number
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
                icon: Object.values(MemoryMixerIcon)[cell?.icon() || 0],
                players_on_card: cell?.playersOnCard() || 0,
                is_highlighted: cell?.isHighlighted() || false
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
