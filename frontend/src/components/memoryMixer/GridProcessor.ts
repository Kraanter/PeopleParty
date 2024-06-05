import type { MemoryMixerGridCell, MemoryMixerGridPayload } from "@/flatbuffers/messageClass"

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
                icon: Object.values(MemoryMixerIcon)[cell?.icon() || 0],
                players_on_card: cell?.playersOnCard() || 0,
                is_highlighted: cell?.isHighlighted() || false
            })
        }
        grid.push(row)
    }
    return {
        timeLeft: Number(payload.timeLeft()),
        maxOnCard: payload.maxOnCard(),
        phase: payload.phase(),
        grid
    }
}
