export function getPlayerSprite(carType: number): string {
    if (carType < 0 || carType > 11) {
        return "/assets/games/highwayHustle/player/player_type_0.png"
    } else {
        return `/assets/games/highwayHustle/player/player_type_${carType}.png`
    }
}

export function getPlayerSpriteDimensions(carType: number): { width: number; height: number } {
    switch (carType) {
        default:
        case 0:
        case 1:
        case 2:
            return { width: 50, height: 28 }
        case 3:
            return { width: 48, height: 29 }
        case 4:
            return { width: 49, height: 27 }
        case 5:
            return { width: 44, height: 26 }
        case 6:
            return { width: 43, height: 26 }
        case 7:
            return { width: 57, height: 30 }
        case 8:
        case 9:
        case 10:
        case 11:
            return { width: 48, height: 26 }
    }
}

export function getObstacleSprite(carType: number): string {
    if (carType < 0 || carType > 14) {
        return "/assets/games/highwayHustle/obstacle/obstacle_type_0.png"
    } else {
        return `/assets/games/highwayHustle/obstacle/obstacle_type_${carType}.png`
    }
}

export function getObstacleDimensions(carType: number): { width: number; height: number } {
    switch (carType) {
        default:
        case 0:
        case 5:
        case 6:
            return { width: 58, height: 31 }
        case 1:
            return { width: 49, height: 27 }
        case 2:
            return { width: 72, height: 35 }
        case 3:
            return { width: 71, height: 35 }
        case 4:
            return { width: 68, height: 35 }
        case 7:
        case 8:
            return { width: 52, height: 30 }
        case 9:
            return { width: 47, height: 27 }
        case 10:
            return { width: 49, height: 28 }
        case 11:
            return { width: 50, height: 28 }
        case 12:
        case 13:
            return { width: 49, height: 29 }
        case 14:
            return { width: 43, height: 26 }
    }
}