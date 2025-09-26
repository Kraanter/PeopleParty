#ifndef PEOPLEPARTY_BACKEND_MARBLE_MANIA_OBSTACLE_H
#define PEOPLEPARTY_BACKEND_MARBLE_MANIA_OBSTACLE_H

#include <string>
#include <vector>
#include <box2d/box2d.h>
#include "../../util/math/vector2d.h"

enum class ObstacleType : uint8_t { Circle = 0, Rectangle = 1, Triangle = 2 };

struct ObstacleMovement {
    // Continuous rotation (radians per second). 0 => no rotation.
    float angularVelocity = 0.0f;

    // Oscillation along a direction (unit direction recommended).
    // speed: units/sec, length: total travel distance (peak-to-peak is 2*length).
    bool oscillate = false;
    Vector2D direction {1.0f, 0.0f};
    float speed = 0.0f;     // units/sec along 'direction'
    float length = 0.0f;    // half-range from start to end (one-way amplitude)
};

class MarbleManiaObstacle {
public:
    MarbleManiaObstacle(b2World& world,
                        const std::string& id,
                        ObstacleType type,
                        const Vector2D& position,
                        float width, float height,
                        float initialRotationRadians);

    ~MarbleManiaObstacle();

    // For frontend
    Vector2D GetPosition() const;  // Returns physics coordinates
    Vector2D GetWorldPosition() const;  // Returns world coordinates (scaled up)
    float GetCurrentRotation() const; // radians
    const std::string& GetId() const { return id_; }
    ObstacleType GetObstacleType() const { return type_; }
    bool IsCircle() const { return type_ == ObstacleType::Circle; }
    float GetWidth() const { return width_; }
    float GetHeight() const { return height_; }
    float GetWorldWidth() const;  // Returns world width (scaled up)
    float GetWorldHeight() const;  // Returns world height (scaled up)

    // Triangle vertices in local space (useful for flatbuffers polygon)
    std::vector<Vector2D> GetTriangleLocalVerts() const;
    std::vector<Vector2D> GetTriangleWorldLocalVerts() const;  // Scaled up for world coords

    b2Body* GetBody() const { return body_; }

private:
    void createBodyAndFixture_(b2World& world,
                               const Vector2D& pos,
                               float rotationRadians);

    std::string id_;
    ObstacleType type_;
    float width_;
    float height_;
    b2Body* body_ = nullptr;
};

#endif // PEOPLEPARTY_BACKEND_MARBLE_MANIA_OBSTACLE_H
