#include "collision_shape.h"
#include <cmath>
#include <algorithm>

// CollisionShape base class
CollisionShape::CollisionShape(const Vector2D& position, float rotation)
    : m_position(position), m_rotation(rotation) {
}

// CircleShape implementation
CircleShape::CircleShape(const Vector2D& center, float radius)
    : CollisionShape(center), m_radius(radius) {
}

CollisionShape* CircleShape::Clone() const {
    return new CircleShape(m_position, m_radius);
}

// RectangleShape implementation
RectangleShape::RectangleShape(const Vector2D& center, float width, float height, float rotation)
    : CollisionShape(center, rotation), m_width(width), m_height(height) {
    UpdateCorners();
}

CollisionShape* RectangleShape::Clone() const {
    return new RectangleShape(m_position, m_width, m_height, m_rotation);
}

void RectangleShape::SetDimensions(float width, float height) {
    m_width = width;
    m_height = height;
    UpdateCorners();
}

void RectangleShape::SetRotation(float rotation) {
    m_rotation = rotation;
    UpdateCorners();
}

void RectangleShape::Rotate(float deltaRotation) {
    m_rotation += deltaRotation;
    UpdateCorners();
}

void RectangleShape::UpdateCorners() {
    float halfWidth = m_width * 0.5f;
    float halfHeight = m_height * 0.5f;
    
    // Define corners in local space (relative to center)
    Vector2D localCorners[4] = {
        Vector2D(-halfWidth, -halfHeight), // Bottom-left
        Vector2D(halfWidth, -halfHeight),  // Bottom-right
        Vector2D(halfWidth, halfHeight),   // Top-right
        Vector2D(-halfWidth, halfHeight)   // Top-left
    };
    
    // Apply rotation to corners
    float cos_r = std::cos(m_rotation);
    float sin_r = std::sin(m_rotation);
    
    for (int i = 0; i < 4; i++) {
        Vector2D& corner = localCorners[i];
        m_corners[i] = Vector2D(
            corner.x * cos_r - corner.y * sin_r,
            corner.x * sin_r + corner.y * cos_r
        );
    }
}

Vector2D RectangleShape::GetCorner(int index) const {
    if (index < 0 || index > 3) return m_position;
    return m_position + m_corners[index];
}

void RectangleShape::GetAllCorners(Vector2D corners[4]) const {
    for (int i = 0; i < 4; i++) {
        corners[i] = m_position + m_corners[i];
    }
}

Vector2D RectangleShape::GetClosestPointToCircle(const Vector2D& circleCenter) const {
    // Transform circle center to rectangle's local space
    Vector2D localCenter = circleCenter - m_position;
    
    // Rotate to align with rectangle's axes
    float cos_r = std::cos(-m_rotation);
    float sin_r = std::sin(-m_rotation);
    Vector2D rotatedCenter(
        localCenter.x * cos_r - localCenter.y * sin_r,
        localCenter.x * sin_r + localCenter.y * cos_r
    );
    
    // Clamp to rectangle bounds
    float halfWidth = m_width * 0.5f;
    float halfHeight = m_height * 0.5f;
    Vector2D closestLocal(
        std::max(-halfWidth, std::min(halfWidth, rotatedCenter.x)),
        std::max(-halfHeight, std::min(halfHeight, rotatedCenter.y))
    );
    
    // Transform back to world space
    Vector2D worldClosest(
        closestLocal.x * cos_r - closestLocal.y * sin_r,
        closestLocal.x * sin_r + closestLocal.y * cos_r
    );
    
    return m_position + worldClosest;
}

bool RectangleShape::ContainsPoint(const Vector2D& point) const {
    // Transform point to rectangle's local space
    Vector2D localPoint = point - m_position;
    
    // Rotate to align with rectangle's axes
    float cos_r = std::cos(-m_rotation);
    float sin_r = std::sin(-m_rotation);
    Vector2D rotatedPoint(
        localPoint.x * cos_r - localPoint.y * sin_r,
        localPoint.x * sin_r + localPoint.y * cos_r
    );
    
    // Check if point is within rectangle bounds
    float halfWidth = m_width * 0.5f;
    float halfHeight = m_height * 0.5f;
    return (rotatedPoint.x >= -halfWidth && rotatedPoint.x <= halfWidth &&
            rotatedPoint.y >= -halfHeight && rotatedPoint.y <= halfHeight);
}