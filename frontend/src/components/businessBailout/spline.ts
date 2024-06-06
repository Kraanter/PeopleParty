// Define a Point interface
interface Point {
  x: number
  y: number
}

// Function to interpolate between two points
function interpolate(p0: Point, p1: Point, t: number): Point {
  return {
    x: (1 - t) * p0.x + t * p1.x,
    y: (1 - t) * p0.y + t * p1.y
  }
}

// De Casteljau's algorithm
export function deCasteljau(points: Point[], t: number): Point {
  if (points.length === 1) {
    return points[0]
  }

  const newPoints: Point[] = []
  for (let i = 0; i < points.length - 1; i++) {
    newPoints.push(interpolate(points[i], points[i + 1], t))
  }

  return deCasteljau(newPoints, t)
}
