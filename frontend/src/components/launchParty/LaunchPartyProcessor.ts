export interface LaunchPartyLights {
    lights: number // 1-4 = red, 5 = all green
}

export interface LauncPartyResultPair {
    player: string
    reaction_time: number
}

export interface LaunchPartyResults {
    results: LauncPartyResultPair[]
}