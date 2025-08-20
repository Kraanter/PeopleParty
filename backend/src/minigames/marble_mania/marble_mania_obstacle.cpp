#include "marble_mania_obstacle.h"
#include <sstream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

MarbleManiaObstacle::MarbleManiaObstacle(PhysicsObject* physicsObject, ObstacleType type, const std::string& id)
    : m_physicsObject(physicsObject), m_type(type), m_id(id)
    , m_originalPosition(physicsObject->GetPosition())
    , m_movementDirection(0, 0), m_movementSpeed(0.0f), m_movementDistance(0.0f), m_movementTimer(0.0f)
    , m_rotationSpeed(0.0f)
    , m_isCircle(false), m_width(40.0f), m_height(40.0f)
{
    CacheShapeProperties();
}

void MarbleManiaObstacle::CacheShapeProperties() {
    if (!m_physicsObject || !m_physicsObject->GetShape()) {
        return;
    }
    
    const CollisionShape* shape = m_physicsObject->GetShape();
    m_isCircle = (shape->GetType() == ShapeType::CIRCLE);
    
    if (m_isCircle) {
        const CircleShape* circle = static_cast<const CircleShape*>(shape);
        float radius = circle->GetRadius();
        m_width = m_height = radius * 2.0f;
    } else {
        const RectangleShape* rect = static_cast<const RectangleShape*>(shape);
        m_width = rect->GetWidth();
        m_height = rect->GetHeight();
    }
}

void MarbleManiaObstacle::SetMovementPattern(const Vector2D& direction, float speed, float distance) {
    m_movementDirection = direction;
    m_movementDirection.Normalize();
    m_movementSpeed = speed;
    m_movementDistance = distance;
    
    // Store the starting position for movement calculations
    m_originalPosition = m_physicsObject->GetPosition();
}

void MarbleManiaObstacle::Update(float deltaTime) {
    UpdateMovement(deltaTime);
    UpdateRotation(deltaTime);
}

void MarbleManiaObstacle::UpdateMovement(float deltaTime) {
    if (m_type != ObstacleType::MOVING_CIRCLE && m_type != ObstacleType::MOVING_RECTANGLE) {
        return;
    }
    
    m_movementTimer += deltaTime;
    
    // Calculate movement using sine wave for smooth back-and-forth motion
    float cycle = m_movementTimer * m_movementSpeed / m_movementDistance;
    float offsetDistance = (m_movementDistance / 2.0f) * sin(cycle * M_PI);
    
    // Calculate new position based on oscillation
    Vector2D newPosition = m_originalPosition + m_movementDirection * offsetDistance;
    
    // Update physics object position
    m_physicsObject->SetPosition(newPosition);
}

void MarbleManiaObstacle::UpdateRotation(float deltaTime) {
    if (m_type != ObstacleType::SPINNING_CIRCLE && m_type != ObstacleType::SPINNING_RECTANGLE) {
        return;
    }
    
    float currentRotation = m_physicsObject->GetRotation();
    currentRotation += m_rotationSpeed * deltaTime;
    
    // Keep rotation in range [0, 2π]
    while (currentRotation >= 2.0f * M_PI) {
        currentRotation -= 2.0f * M_PI;
    }
    while (currentRotation < 0.0f) {
        currentRotation += 2.0f * M_PI;
    }
    
    m_physicsObject->SetRotation(currentRotation);
}
