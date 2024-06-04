import type { MemoryMixerGridPayload } from "@/flatbuffers/messageClass"
//import { getRandomInt } from './utils'; // Import the getRandomInt function

function getRandomInt(min, max) {
    const minCeiled = Math.ceil(min);
    const maxFloored = Math.floor(max);
    return Math.floor(Math.random() * (maxFloored - minCeiled) + minCeiled); // The maximum is exclusive and the minimum is inclusive
};

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

/*export const processGrid = (payload: MemoryMixerGridPayload): MemoryMixerGrid => {
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
}*/

export const processGrid = (payload: MemoryMixerGridPayload): MemoryMixerGrid => {
  const grid: MemoryMixerCell[][] = [];
  for (let i = 0; i < payload.gridLength(); i++) {
    const row: MemoryMixerCell[] = [];
    for (let j = 0; j < payload.grid(i)?.rowLength()!; j++) {
      const cell = payload.grid(i)?.row(j);
      //const randomIcon = getRandomInt(0, Object.keys(MemoryMixerIcon).length - 2); // Get a random icon index
      const values = Object.keys(MemoryMixerIcon);
      const randomIcon = values[Math.floor(Math.random() * values.length)];
      row.push({
        icon: MemoryMixerIcon[randomIcon], // Use the random icon
        players_on_card: cell?.playersOnCard() || 0
      });
    }
    grid.push(row);
  }

  return {
    timeLeft: Number(payload.timeLeft()),
    maxOnCard: Number(payload.maxOnCard()),
    grid
  };
};
