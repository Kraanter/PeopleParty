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
  console.log(matchCount)

  for (let i = 0; i < matchCount; i++) {
    matches.push({})
  }

  const playerNames = Array.from({ length: playerCount }, (_, i) => `Player ${i}`)

  const round1Count = Math.ceil(matchCount / 2)

  // Seed the players into the first round of the bracket
  for (let i = 0; i < round1Count; i++) {
    const leftName = playerNames[i]
    const rightName = playerNames[i + round1Count]
    const left = { name: leftName, move: null }
    const right = rightName ? { name: rightName, move: null } : undefined
    matches[round1Count + i] = {
      left,
      right
    }
  }

  // Set winners of the first round
  for (let i = 0; i < round1Count; i++) {
    const left = matches[round1Count + i].left ? 'left' : 'right'
    const right = matches[round1Count + i].right ? 'right' : 'left'
    const winner = Math.random() < 0.5 ? left : right
    matches[round1Count + i].winner = winner
  }

  return { matches }
}
