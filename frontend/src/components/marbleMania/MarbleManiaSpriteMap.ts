// Sprite mapping for MarbleMania entities

export interface MarbleSpriteDimensions {
  width: number
  height: number
}

export interface ObstacleSpriteDimensions {
  width: number
  height: number
}

export function getMarbleSprite(marbleType: number = 0): string {
  switch (marbleType) {
    case 0: // Normal marble
      return '/assets/games/marbleMania/marble-normal.png'
    case 1: // Heavy marble
      return '/assets/games/marbleMania/marble-heavy.png'
    case 2: // Light marble
      return '/assets/games/marbleMania/marble-light.png'
    case 3: // Bouncy marble
      return '/assets/games/marbleMania/marble-bouncy.png'
    case 4: // Sticky marble
      return '/assets/games/marbleMania/marble-sticky.png'
    default:
      return '/assets/games/marbleMania/marble-normal.png'
  }
}

export function getMarbleSpriteDimensions(marbleType: number = 0): MarbleSpriteDimensions {
  switch (marbleType) {
    case 0: // Normal marble
      return { width: 30, height: 30 }
    case 1: // Heavy marble (larger)
      return { width: 36, height: 36 }
    case 2: // Light marble (smaller)
      return { width: 24, height: 24 }
    case 3: // Bouncy marble
      return { width: 30, height: 30 }
    case 4: // Sticky marble
      return { width: 33, height: 33 }
    default:
      return { width: 30, height: 30 }
  }
}

export function getObstacleSprite(obstacleType: number = 0): string {
  switch (obstacleType) {
    case 0: // Static circle
      return '/assets/games/marbleMania/obstacle-circle.png'
    case 1: // Static rectangle
      return '/assets/games/marbleMania/obstacle-rectangle.png'
    case 2: // Moving circle
      return '/assets/games/marbleMania/obstacle-circle-moving.png'
    case 3: // Moving rectangle
      return '/assets/games/marbleMania/obstacle-rectangle-moving.png'
    default:
      return '/assets/games/marbleMania/obstacle-circle.png'
  }
}

export function getObstacleSpriteDimensions(obstacleType: number = 0): ObstacleSpriteDimensions {
  switch (obstacleType) {
    case 0: // Static circle
      return { width: 50, height: 50 }
    case 1: // Static rectangle
      return { width: 100, height: 20 }
    case 2: // Moving circle
      return { width: 30, height: 30 }
    case 3: // Moving rectangle
      return { width: 60, height: 15 }
    default:
      return { width: 50, height: 50 }
  }
}

export function getMarbleColor(marbleType: number = 0): number {
  switch (marbleType) {
    case 0: // Normal marble
      return 0x3366ff // Blue
    case 1: // Heavy marble
      return 0x808080 // Gray
    case 2: // Light marble
      return 0xffffff // White
    case 3: // Bouncy marble
      return 0x00ff00 // Green
    case 4: // Sticky marble
      return 0xff8000 // Orange
    default:
      return 0x3366ff // Blue
  }
}

export function getObstacleColor(obstacleType: number = 0): number {
  switch (obstacleType) {
    case 0: // Static circle
      return 0x8b4513 // Brown
    case 1: // Static rectangle
      return 0x654321 // Dark brown
    case 2: // Moving circle
      return 0xff6b6b // Red
    case 3: // Moving rectangle
      return 0xff4757 // Dark red
    default:
      return 0x8b4513 // Brown
  }
}
