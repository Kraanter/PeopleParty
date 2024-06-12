export type PlayerRPS = {
  name: string
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
  console.log('matchCount', matchCount)

  for (let i = 0; i < matchCount; i++) {
    matches.push({})
  }

  const playerNames = Array.from({ length: playerCount }, (_, i) => `Player ${i}`)
  console.log(playerNames)

  // const startRoundIndex = Math.floor(matchCount / 2)
  const startRoundIndex = 1

  // Seed the players into the first round of the bracket
  for (let i = 0; i < matchCount - startRoundIndex; i++) {
    console.log(i, matchCount - i, i)
    const leftName = playerNames[i]
    const rightName = playerNames[matchCount - i]
    const left = { name: leftName, move: null }
    const right =
      rightName && i !== playerNames.length - i ? { name: rightName, move: null } : undefined
    matches[startRoundIndex + i] = {
      left,
      right
    }
  }

  matches[0] = {
    left: matches[startRoundIndex].left,
    right: matches[startRoundIndex].right
  }

  return { matches }
}
