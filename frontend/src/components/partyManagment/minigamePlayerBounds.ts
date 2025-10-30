export const minigamePlayerBoundExceptions: { name: string, min: number, max: number }[] = [
    { name: "rps_bracket", min: 2, max: 64 },
    { name: "highway_hustle", min: 2, max: 12 },
    { name: "memory_mixer", min: 2, max: -1 },
]

// default min = 1
// default max = -1 (no limit)
// you only have to place exceptions in here