export interface RpsBracketPlayer {
  name: string
}

export interface RpsBracketMatch {
  left: RpsBracketPlayer
  right: RpsBracketPlayer
  winner: RpsBracketPlayer
}

export interface RpsBracketHostData {
  matches: RpsBracketMatch[]
}
