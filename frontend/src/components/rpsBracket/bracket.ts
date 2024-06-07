export type PlayerRPS = {
  name: string
  move: 'rock' | 'paper' | 'scissors' | null
}

export type BracketMatch = {
  left?: PlayerRPS
  right?: PlayerRPS
  winner?: 'left' | 'right'
}

export type Bracket = {
  matches: BracketMatch[]
}

export function createBracket(playerCount: number): Bracket {
  // First index is final match, second 2 indexes are semi-final matches, etc.
  const matches: BracketMatch[] = []
  const matchCount = Math.pow(2, Math.ceil(Math.log2(playerCount))) - 1
  playerCount = matchCount

  for (let i = 0; i < matchCount; i++) {
    matches.push({})
  }

  const playerNames = Array.from({ length: playerCount + 2 }, (_, i) => `Player ${i}`)

  // Seed the players into the first round of the bracket
  for (let i = 0; i < matchCount; i++) {
    const leftName = playerNames[i]
    const rightName = playerNames[i + 1]
    const left = { name: leftName, move: null }
    const right = rightName ? { name: rightName, move: null } : undefined
    matches[i] = {
      left,
      right,
      winner: Math.random() < 0.5 ? 'left' : 'right'
    }
  }

  return { matches }
}
