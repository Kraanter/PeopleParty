#include "marble_mania_obstacle.h"
#include <sstream>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

MarbleManiaObstacle::MarbleManiaObstacle(const Vector2D& position, ObstacleType type, bool isCircle)
    : RigidBody(position, 0.0f, 20.0f) // Static obstacles have 0 mass
    , m_type(type)
    , m_originalPosition(position)
    , m_movementDirection(0, 0)
    , m_movementSpeed(0.0f)
    , m_movementDistance(0.0f)
    , m_movementTimer(0.0f)
    , m_movingForward(true)
    , m_rotationSpeed(0.0f)
    , m_currentRotation(0.0f)
    , m_isCircle(isCircle)
    , m_width(40.0f)
    , m_height(40.0f)
{
    // Static obstacles don't move
    SetStatic(true);
    
    // Generate unique ID
    std::stringstream ss;
    ss << "obstacle_" << (int)position.x << "_" << (int)position.y << "_" << (int)type;
    m_id = ss.str();
    
    // Set restitution for bouncy obstacles
    SetRestitution(0.8f);
    SetFriction(0.3f);
}

void MarbleManiaObstacle::SetMovementPattern(const Vector2D& direction, float speed, float distance) {
    m_movementDirection = direction;
    m_movementDirection.Normalize();
    m_movementSpeed = speed;
    m_movementDistance = distance;
    
    // Moving obstacles are not static
    if (m_type == ObstacleType::MOVING_CIRCLE || m_type == ObstacleType::MOVING_RECTANGLE) {
        SetStatic(false);
        SetMass(100.0f); // Heavy so they don't get pushed around by marbles
    }
}

void MarbleManiaObstacle::UpdateMovement(float deltaTime) {
    if (m_type != ObstacleType::MOVING_CIRCLE && m_type != ObstacleType::MOVING_RECTANGLE) {
        return;
    }
    
    m_movementTimer += deltaTime;
    
    // Calculate movement using sine wave for smooth back-and-forth motion
    float cycle = m_movementTimer * m_movementSpeed / m_movementDistance; // Speed controls cycle frequency
    float offsetDistance = (m_movementDistance / 2.0f) * sin(cycle * M_PI); // Oscillate between -distance/2 and +distance/2
    
    // Calculate new position based on oscillation
    Vector2D newPosition = m_originalPosition + m_movementDirection * offsetDistance;
    
    // Update position
    SetPosition(newPosition);
}

void MarbleManiaObstacle::UpdateRotation(float deltaTime) {
    if (m_type != ObstacleType::SPINNING_CIRCLE && m_type != ObstacleType::SPINNING_RECTANGLE) {
        return;
    }
    
    m_currentRotation += m_rotationSpeed * deltaTime;
    
    // Keep rotation in range [0, 2π]
    while (m_currentRotation >= 2.0f * M_PI) {
        m_currentRotation -= 2.0f * M_PI;
    }
    while (m_currentRotation < 0.0f) {
        m_currentRotation += 2.0f * M_PI;
    }
}

void MarbleManiaObstacle::Update(float deltaTime) {
    UpdateMovement(deltaTime);
    UpdateRotation(deltaTime);
}

bool MarbleManiaObstacle::CheckCollisionWithCircle(const Vector2D& circleCenter, float circleRadius) const {
    if (m_isCircle) {
        // Simple circle-circle collision
        float distance = GetPosition().Distance(circleCenter);
        return distance < (GetRadius() + circleRadius);
    } else {
        // Rectangle-circle collision (accounting for rotation)
        Vector2D rectCenter = GetPosition();
        float rotation = GetCurrentRotation();
        
        // Transform circle center to rectangle's local coordinate system
        Vector2D localCenter = circleCenter - rectCenter;
        
        // Rotate the circle center by negative rotation to align with rectangle
        float cos_rot = cos(-rotation);
        float sin_rot = sin(-rotation);
        Vector2D rotatedCenter = Vector2D(
            localCenter.x * cos_rot - localCenter.y * sin_rot,
            localCenter.x * sin_rot + localCenter.y * cos_rot
        );
        
        // Check collision with axis-aligned rectangle
        float halfWidth = m_width / 2;
        float halfHeight = m_height / 2;
        
        // Find closest point on rectangle to circle center
        float closestX = std::max(-halfWidth, std::min(halfWidth, rotatedCenter.x));
        float closestY = std::max(-halfHeight, std::min(halfHeight, rotatedCenter.y));
        
        // Calculate distance from circle center to closest point
        float dx = rotatedCenter.x - closestX;
        float dy = rotatedCenter.y - closestY;
        float distanceSquared = dx * dx + dy * dy;
        
        return distanceSquared < (circleRadius * circleRadius);
    }
}

MarbleManiaObstacle* MarbleManiaObstacle::CreateStaticCircle(const Vector2D& position, float radius) {
    MarbleManiaObstacle* obstacle = new MarbleManiaObstacle(position, ObstacleType::STATIC_CIRCLE, true);
    obstacle->SetRadius(radius);
    obstacle->SetDimensions(radius * 2, radius * 2);
    return obstacle;
}

MarbleManiaObstacle* MarbleManiaObstacle::CreateStaticRectangle(const Vector2D& position, float width, float height) {
    MarbleManiaObstacle* obstacle = new MarbleManiaObstacle(position, ObstacleType::STATIC_RECTANGLE, false);
    obstacle->SetRadius(sqrt(width * width + height * height) / 2); // Approximate radius for broad-phase
    obstacle->SetDimensions(width, height);
    return obstacle;
}

MarbleManiaObstacle* MarbleManiaObstacle::CreateMovingCircle(const Vector2D& position, float radius,
                                                           const Vector2D& direction, float speed, float distance) {
    MarbleManiaObstacle* obstacle = new MarbleManiaObstacle(position, ObstacleType::MOVING_CIRCLE, true);
    obstacle->SetRadius(radius);
    obstacle->SetDimensions(radius * 2, radius * 2);
    obstacle->SetMovementPattern(direction, speed, distance);
    return obstacle;
}

MarbleManiaObstacle* MarbleManiaObstacle::CreateMovingRectangle(const Vector2D& position, float width, float height,
                                                              const Vector2D& direction, float speed, float distance) {
    MarbleManiaObstacle* obstacle = new MarbleManiaObstacle(position, ObstacleType::MOVING_RECTANGLE, false);
    obstacle->SetRadius(sqrt(width * width + height * height) / 2); // Approximate radius for broad-phase
    obstacle->SetDimensions(width, height);
    obstacle->SetMovementPattern(direction, speed, distance);
    return obstacle;
}

MarbleManiaObstacle* MarbleManiaObstacle::CreateSpinningCircle(const Vector2D& position, float radius, float rotationSpeed) {
    MarbleManiaObstacle* obstacle = new MarbleManiaObstacle(position, ObstacleType::SPINNING_CIRCLE, true);
    obstacle->SetRadius(radius);
    obstacle->SetDimensions(radius * 2, radius * 2);
    obstacle->SetRotationSpeed(rotationSpeed);
    return obstacle;
}

MarbleManiaObstacle* MarbleManiaObstacle::CreateSpinningRectangle(const Vector2D& position, float width, float height, float rotationSpeed) {
    MarbleManiaObstacle* obstacle = new MarbleManiaObstacle(position, ObstacleType::SPINNING_RECTANGLE, false);
    
    // For thin rectangles, use a much smaller collision radius
    float aspectRatio = std::max(width, height) / std::min(width, height);
    
    if (aspectRatio > 5.0f) {
        // Very thin rectangle - use thickness plus small margin as radius
        obstacle->SetRadius(std::min(width, height) / 2 + 5.0f);
    } else {
        // More square rectangle - use standard calculation
        float diagonalRadius = sqrt(width * width + height * height) / 2;
        float maxDimensionRadius = std::max(width, height) / 2;
        obstacle->SetRadius(std::min(diagonalRadius, maxDimensionRadius * 1.2f));
    }
    
    obstacle->SetDimensions(width, height);
    obstacle->SetRotationSpeed(rotationSpeed);
    return obstacle;
}
