export type PlayerRPS = {
  name: string
  move: 'rock' | 'paper' | 'scissors' | null
}

export type BracketMatch = {
  left?: PlayerRPS
  right?: PlayerRPS
  winner?: PlayerRPS
}

export type Bracket = {
  matches: BracketMatch[]
}

export function createBracket(playerCount: number): Bracket {
  // First index is final match, second 2 indexes are semi-final matches, etc.
  const matches: BracketMatch[] = []
  const matchCount = Math.pow(2, Math.ceil(Math.log2(playerCount))) - 1

  for (let i = 0; i < matchCount; i++) {
    matches.push({})
  }

  const playerNames = Array.from({ length: playerCount }, (_, i) => `Player ${i}`)

  const startRoundIndex = Math.ceil(matchCount / 2)

  // Seed the players into the first round of the bracket
  for (let i = 0; i < startRoundIndex; i++) {
    const leftName = playerNames[i]
    const rightName = playerNames[i + startRoundIndex]
    const left = { name: leftName, move: null }
    const right = rightName ? { name: rightName, move: null } : undefined
    matches[startRoundIndex + i] = {
      left,
      right
    }
  }

  matches[matches.length - 1] = {}

  return { matches }
}
