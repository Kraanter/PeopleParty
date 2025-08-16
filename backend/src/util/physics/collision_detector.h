#ifndef COLLISION_DETECTOR_H
#define COLLISION_DETECTOR_H

#include "collision_shape.h"
#include "physics_object.h"
#include "../math/vector2d.h"

// Structure to hold collision information
struct CollisionInfo {
    bool hasCollision;
    Vector2D normal;          // Collision normal (pointing away from first object)
    float penetrationDepth;   // How much the objects are overlapping
    Vector2D contactPoint;    // Point of contact in world space
    
    CollisionInfo() : hasCollision(false), penetrationDepth(0.0f) {}
    CollisionInfo(const Vector2D& norm, float depth, const Vector2D& contact)
        : hasCollision(true), normal(norm), penetrationDepth(depth), contactPoint(contact) {}
};

// Static class for collision detection between different shape types
class CollisionDetector {
public:
    // Main collision detection method
    static CollisionInfo DetectCollision(const PhysicsObject& objectA, const PhysicsObject& objectB);
    static CollisionInfo DetectCollision(const CollisionShape& shapeA, const CollisionShape& shapeB);
    
    // Specific collision detection methods
    static CollisionInfo CircleVsCircle(const CircleShape& circleA, const CircleShape& circleB);
    static CollisionInfo CircleVsRectangle(const CircleShape& circle, const RectangleShape& rectangle);
    static CollisionInfo RectangleVsCircle(const RectangleShape& rectangle, const CircleShape& circle);
    static CollisionInfo RectangleVsRectangle(const RectangleShape& rectA, const RectangleShape& rectB);
    
private:
    // Helper methods
    static bool IsPointInRectangle(const Vector2D& point, const RectangleShape& rectangle);
    static Vector2D GetClosestPointOnRectangle(const Vector2D& point, const RectangleShape& rectangle);
    static float DistancePointToLine(const Vector2D& point, const Vector2D& lineStart, const Vector2D& lineEnd);
    static Vector2D ProjectPointOntoLine(const Vector2D& point, const Vector2D& lineStart, const Vector2D& lineEnd);
    
    // SAT (Separating Axis Theorem) for rectangle vs rectangle
    static bool SATCollision(const RectangleShape& rectA, const RectangleShape& rectB, CollisionInfo& info);
    static Vector2D GetRectangleAxis(const RectangleShape& rectangle, int axisIndex);
    static void ProjectRectangleOntoAxis(const RectangleShape& rectangle, const Vector2D& axis, float& min, float& max);
};

#endif // COLLISION_DETECTOR_H