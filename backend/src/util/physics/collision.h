#ifndef COLLISION_H
#define COLLISION_H

#include "../math/vector2d.h"
#include "rigid_body.h"

struct CollisionInfo {
    bool hasCollision;
    Vector2D contactPoint;
    Vector2D normal;
    float penetrationDepth;
    
    CollisionInfo() : hasCollision(false), contactPoint(0, 0), normal(0, 0), penetrationDepth(0) {}
};

class Collision {
public:
    // Circle-Circle collision detection
    static CollisionInfo CheckCircleCircle(const RigidBody& body1, const RigidBody& body2);
    
    // Circle-Line collision detection (for walls/boundaries)
    static CollisionInfo CheckCircleLine(const RigidBody& body, const Vector2D& lineStart, const Vector2D& lineEnd);
    
    // Circle-Rectangle collision detection (for rectangular boundaries)
    static CollisionInfo CheckCircleRectangle(const RigidBody& body, const Vector2D& rectMin, const Vector2D& rectMax);
    
    // Resolve collision between two rigid bodies
    static void ResolveCollision(RigidBody& body1, RigidBody& body2, const CollisionInfo& collision);
    
    // Resolve collision between a rigid body and a static boundary
    static void ResolveStaticCollision(RigidBody& body, const CollisionInfo& collision);
    
    // Separate overlapping bodies
    static void SeparateBodies(RigidBody& body1, RigidBody& body2, const CollisionInfo& collision);
    
    // Apply friction during collision
    static void ApplyFriction(RigidBody& body1, RigidBody& body2, const Vector2D& normal, float frictionCoeff);
    
private:
    // Helper functions
    static Vector2D GetClosestPointOnLine(const Vector2D& point, const Vector2D& lineStart, const Vector2D& lineEnd);
    static Vector2D ClampVector(const Vector2D& vector, const Vector2D& min, const Vector2D& max);
};

#endif // COLLISION_H
