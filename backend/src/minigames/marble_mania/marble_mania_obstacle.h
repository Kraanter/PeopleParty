#ifndef MARBLE_MANIA_OBSTACLE_H
#define MARBLE_MANIA_OBSTACLE_H

#include "../../util/physics/rigid_body.h"
#include "../../util/math/vector2d.h"
#include <string>

enum class ObstacleType {
    STATIC_CIRCLE,
    STATIC_RECTANGLE,
    MOVING_CIRCLE,
    MOVING_RECTANGLE
};

class MarbleManiaObstacle : public RigidBody {
private:
    ObstacleType m_type;
    std::string m_id;
    
    // For moving obstacles
    Vector2D m_originalPosition;
    Vector2D m_movementDirection;
    float m_movementSpeed;
    float m_movementDistance;
    float m_movementTimer;
    bool m_movingForward;
    
    // Shape properties
    bool m_isCircle;
    float m_width;
    float m_height;
    
public:
    MarbleManiaObstacle(const Vector2D& position, ObstacleType type, bool isCircle = true);
    
    // Type management
    ObstacleType GetObstacleType() const { return m_type; }
    void SetId(const std::string& id) { m_id = id; }
    const std::string& GetId() const { return m_id; }
    
    // Shape properties
    bool IsCircle() const { return m_isCircle; }
    void SetDimensions(float width, float height) { m_width = width; m_height = height; }
    float GetWidth() const { return m_width; }
    float GetHeight() const { return m_height; }
    
    // Movement properties (for moving obstacles)
    void SetMovementPattern(const Vector2D& direction, float speed, float distance);
    void UpdateMovement(float deltaTime);
    
    // Factory methods
    static MarbleManiaObstacle* CreateStaticCircle(const Vector2D& position, float radius);
    static MarbleManiaObstacle* CreateStaticRectangle(const Vector2D& position, float width, float height);
    static MarbleManiaObstacle* CreateMovingCircle(const Vector2D& position, float radius, 
                                                  const Vector2D& direction, float speed, float distance);
    static MarbleManiaObstacle* CreateMovingRectangle(const Vector2D& position, float width, float height,
                                                     const Vector2D& direction, float speed, float distance);
};

#endif // MARBLE_MANIA_OBSTACLE_H
