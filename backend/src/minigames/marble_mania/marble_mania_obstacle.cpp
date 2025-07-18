#include "marble_mania_obstacle.h"
#include <sstream>
#include <cmath>

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
    
    // Calculate how far we should have moved
    float targetDistance = m_movementSpeed * m_movementTimer;
    
    // Check if we need to reverse direction
    if (targetDistance >= m_movementDistance) {
        m_movingForward = !m_movingForward;
        m_movementTimer = 0.0f;
        targetDistance = 0.0f;
    }
    
    // Calculate new position
    Vector2D direction = m_movingForward ? m_movementDirection : m_movementDirection * -1;
    Vector2D newPosition = m_originalPosition + direction * targetDistance;
    
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
    obstacle->SetRadius(sqrt(width * width + height * height) / 2); // Approximate radius for broad-phase
    obstacle->SetDimensions(width, height);
    obstacle->SetRotationSpeed(rotationSpeed);
    return obstacle;
}
