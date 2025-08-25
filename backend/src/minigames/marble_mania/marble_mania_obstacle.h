#ifndef MARBLE_MANIA_OBSTACLE_H
#define MARBLE_MANIA_OBSTACLE_H

#include "../../util/math/vector2d.h"
#include <box2d/box2d.h>
#include <string>

enum class ObstacleType {
    STATIC_CIRCLE = 0,
    STATIC_RECTANGLE = 1,
    MOVING_CIRCLE = 2,
    MOVING_RECTANGLE = 3,
    SPINNING_CIRCLE = 4,
    SPINNING_RECTANGLE = 5
};

class MarbleManiaObstacle {
private:
    b2Body* m_body; // Owned by the Box2D world
    ObstacleType m_type;
    std::string m_id;
    
    // For moving obstacles
    Vector2D m_originalPosition;
    Vector2D m_movementDirection;
    float m_movementSpeed;
    float m_movementDistance;
    float m_movementTimer;
    
    // For spinning obstacles
    float m_rotationSpeed;
    
    // Shape properties (cached for easy access)
    bool m_isCircle;
    float m_width;
    float m_height;
    
public:
    MarbleManiaObstacle(b2Body* body, ObstacleType type, const std::string& id);
    ~MarbleManiaObstacle() = default;
    
    // Basic properties
    ObstacleType GetObstacleType() const { return m_type; }
    const std::string& GetId() const { return m_id; }
    
    // Physics object access
    b2Body* GetBody() { return m_body; }
    const b2Body* GetBody() const { return m_body; }
    
    // Position and transform
    Vector2D GetPosition() const { 
        b2Vec2 pos = m_body->GetPosition();
        return Vector2D(pos.x * 100.0f, pos.y * 100.0f); // Convert from meters to pixels
    }
    void SetPosition(const Vector2D& position) { 
        m_body->SetTransform(b2Vec2(position.x / 100.0f, position.y / 100.0f), m_body->GetAngle());
    }
    
    float GetCurrentRotation() const { return m_body->GetAngle(); }
    void SetRotation(float rotation) { 
        m_body->SetTransform(m_body->GetPosition(), rotation);
    }
    
    // Shape properties
    bool IsCircle() const { return m_isCircle; }
    float GetWidth() const { return m_width * 100.0f; } // Convert from meters to pixels
    float GetHeight() const { return m_height * 100.0f; } // Convert from meters to pixels
    
    float GetRadius() const {
        if (m_isCircle) return (m_width * 100.0f) / 2.0f; // Convert from meters to pixels
        return std::max(m_width, m_height) * 100.0f / 2.0f; // Approximation for rectangles
    }
    
    // Movement properties (for moving obstacles)
    void SetMovementPattern(const Vector2D& direction, float speed, float distance);
    const Vector2D& GetMovementDirection() const { return m_movementDirection; }
    float GetMovementSpeed() const { return m_movementSpeed; }
    float GetMovementDistance() const { return m_movementDistance; }
    
    // Rotation properties (for spinning obstacles)
    void SetRotationSpeed(float speed) { m_rotationSpeed = speed; }
    float GetRotationSpeed() const { return m_rotationSpeed; }
    
    // Update methods
    void Update(float deltaTime);
    
private:
    void UpdateMovement(float deltaTime);
    void UpdateRotation(float deltaTime);
    void CacheShapeProperties();
};

#endif // MARBLE_MANIA_OBSTACLE_H
