#ifndef COLLISION_SHAPE_H
#define COLLISION_SHAPE_H

#include "../math/vector2d.h"

enum class ShapeType {
    CIRCLE,
    RECTANGLE
};

// Base class for all collision shapes
class CollisionShape {
protected:
    Vector2D m_position;
    float m_rotation; // in radians
    
public:
    CollisionShape(const Vector2D& position, float rotation = 0.0f);
    virtual ~CollisionShape() = default;
    
    // Pure virtual methods
    virtual ShapeType GetType() const = 0;
    virtual CollisionShape* Clone() const = 0;
    
    // Position and rotation
    const Vector2D& GetPosition() const { return m_position; }
    void SetPosition(const Vector2D& position) { m_position = position; }
    
    float GetRotation() const { return m_rotation; }
    virtual void SetRotation(float rotation) { m_rotation = rotation; }
    
    // Transform methods
    virtual Vector2D GetCenter() const { return m_position; }
    virtual void Move(const Vector2D& offset) { m_position += offset; }
    virtual void Rotate(float deltaRotation) { m_rotation += deltaRotation; }
};

// Circle collision shape
class CircleShape : public CollisionShape {
private:
    float m_radius;
    
public:
    CircleShape(const Vector2D& center, float radius);
    
    ShapeType GetType() const override { return ShapeType::CIRCLE; }
    CollisionShape* Clone() const override;
    
    float GetRadius() const { return m_radius; }
    void SetRadius(float radius) { m_radius = radius; }
};

// Rectangle collision shape (stores 4 corners for easy rotation handling)
class RectangleShape : public CollisionShape {
private:
    float m_width;
    float m_height;
    Vector2D m_corners[4]; // Local coordinates relative to center
    
    void UpdateCorners();
    
public:
    RectangleShape(const Vector2D& center, float width, float height, float rotation = 0.0f);
    
    ShapeType GetType() const override { return ShapeType::RECTANGLE; }
    CollisionShape* Clone() const override;
    
    float GetWidth() const { return m_width; }
    float GetHeight() const { return m_height; }
    void SetDimensions(float width, float height);
    
    // Get world coordinates of corners
    Vector2D GetCorner(int index) const; // index 0-3
    void GetAllCorners(Vector2D corners[4]) const;
    
    // Override rotation to update corners
    void SetRotation(float rotation) override;
    void Rotate(float deltaRotation) override;
    
    // Helper methods for collision detection
    Vector2D GetClosestPointToCircle(const Vector2D& circleCenter) const;
    bool ContainsPoint(const Vector2D& point) const;
};

#endif // COLLISION_SHAPE_H