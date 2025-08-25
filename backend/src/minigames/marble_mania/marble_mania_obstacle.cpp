#include "marble_mania_obstacle.h"
#include <sstream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

MarbleManiaObstacle::MarbleManiaObstacle(b2Body* body, ObstacleType type, const std::string& id)
    : m_body(body), m_type(type), m_id(id)
    , m_originalPosition(GetPosition())
    , m_movementDirection(0, 0), m_movementSpeed(0.0f), m_movementDistance(0.0f), m_movementTimer(0.0f)
    , m_rotationSpeed(0.0f)
    , m_isCircle(false), m_width(40.0f), m_height(40.0f)
{
    CacheShapeProperties();
}

void MarbleManiaObstacle::CacheShapeProperties() {
    if (!m_body || !m_body->GetFixtureList()) {
        return;
    }
    
    b2Fixture* fixture = m_body->GetFixtureList();
    const b2Shape* shape = fixture->GetShape();
    
    if (shape->GetType() == b2Shape::e_circle) {
        m_isCircle = true;
        const b2CircleShape* circle = static_cast<const b2CircleShape*>(shape);
        float radius = circle->m_radius;
        m_width = m_height = radius * 2.0f;
    } else if (shape->GetType() == b2Shape::e_polygon) {
        m_isCircle = false;
        const b2PolygonShape* poly = static_cast<const b2PolygonShape*>(shape);
        
        // Calculate bounding box for width/height
        b2Vec2 min = poly->m_vertices[0];
        b2Vec2 max = poly->m_vertices[0];
        
        for (int i = 1; i < poly->m_count; i++) {
            if (poly->m_vertices[i].x < min.x) min.x = poly->m_vertices[i].x;
            if (poly->m_vertices[i].y < min.y) min.y = poly->m_vertices[i].y;
            if (poly->m_vertices[i].x > max.x) max.x = poly->m_vertices[i].x;
            if (poly->m_vertices[i].y > max.y) max.y = poly->m_vertices[i].y;
        }
        
        m_width = max.x - min.x;
        m_height = max.y - min.y;
    }
}

void MarbleManiaObstacle::SetMovementPattern(const Vector2D& direction, float speed, float distance) {
    m_movementDirection = direction;
    m_movementDirection.Normalize();
    m_movementSpeed = speed;
    m_movementDistance = distance;
    
    // Store the starting position for movement calculations (in pixels)
    m_originalPosition = GetPosition();
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
    SetPosition(newPosition);
}

void MarbleManiaObstacle::UpdateRotation(float deltaTime) {
    if (m_type != ObstacleType::SPINNING_CIRCLE && m_type != ObstacleType::SPINNING_RECTANGLE) {
        return;
    }
    
    float currentRotation = GetCurrentRotation();
    currentRotation += m_rotationSpeed * deltaTime;
    
    // Keep rotation in range [0, 2π]
    while (currentRotation >= 2.0f * M_PI) {
        currentRotation -= 2.0f * M_PI;
    }
    while (currentRotation < 0.0f) {
        currentRotation += 2.0f * M_PI;
    }
    
    SetRotation(currentRotation);
}
