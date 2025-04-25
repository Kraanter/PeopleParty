export function getPlayerSprite(carType: number): string {
    // todo: fill correct sprite references
    switch (carType) {
        case 0:
            return "car1"
        case 1:
            return "car2"
        case 2:
            return "car3"
        case 3:
            return "car4"
        default:
            return "car1"
    }

}

export function getObstacleSprite(carType: number): string {
    // todo: fill correct sprite references
    switch (carType) {
        case 0:
            return "car1"
        case 1:
            return "car2"
        case 2:
            return "car3"
        case 3:
            return "car4"
        default:
            return "car1"
    }
}