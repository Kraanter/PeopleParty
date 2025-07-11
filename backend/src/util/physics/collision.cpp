#include "collision.h"
#include <cmath>
#include <algorithm>

CollisionInfo Collision::CheckCircleCircle(const RigidBody& body1, const RigidBody& body2) {
    CollisionInfo info;
    
    Vector2D direction = body2.GetPosition() - body1.GetPosition();
    float distance = direction.Length();
    float minDistance = body1.GetRadius() + body2.GetRadius();
    
    if (distance <= minDistance && distance > 0) {
        info.hasCollision = true;
        info.normal = direction / distance; // Normalize
        info.penetrationDepth = minDistance - distance;
        info.contactPoint = body1.GetPosition() + info.normal * body1.GetRadius();
    }
    
    return info;
}

CollisionInfo Collision::CheckCircleLine(const RigidBody& body, const Vector2D& lineStart, const Vector2D& lineEnd) {
    CollisionInfo info;
    
    Vector2D closestPoint = GetClosestPointOnLine(body.GetPosition(), lineStart, lineEnd);
    Vector2D direction = body.GetPosition() - closestPoint;
    float distance = direction.Length();
    
    if (distance <= body.GetRadius() && distance > 0) {
        info.hasCollision = true;
        info.normal = direction / distance; // Normalize
        info.penetrationDepth = body.GetRadius() - distance;
        info.contactPoint = closestPoint;
    }
    
    return info;
}

CollisionInfo Collision::CheckCircleRectangle(const RigidBody& body, const Vector2D& rectMin, const Vector2D& rectMax) {
    CollisionInfo info;
    
    Vector2D closestPoint = ClampVector(body.GetPosition(), rectMin, rectMax);
    Vector2D direction = body.GetPosition() - closestPoint;
    float distance = direction.Length();
    
    if (distance <= body.GetRadius()) {
        info.hasCollision = true;
        
        if (distance > 0) {
            info.normal = direction / distance; // Normalize
            info.penetrationDepth = body.GetRadius() - distance;
        } else {
            // Handle case where circle center is exactly on the rectangle
            // Find the closest edge
            float leftDist = body.GetPosition().x - rectMin.x;
            float rightDist = rectMax.x - body.GetPosition().x;
            float topDist = body.GetPosition().y - rectMin.y;
            float bottomDist = rectMax.y - body.GetPosition().y;
            
            float minDist = std::min({leftDist, rightDist, topDist, bottomDist});
            
            if (minDist == leftDist) {
                info.normal = Vector2D(-1, 0);
            } else if (minDist == rightDist) {
                info.normal = Vector2D(1, 0);
            } else if (minDist == topDist) {
                info.normal = Vector2D(0, -1);
            } else {
                info.normal = Vector2D(0, 1);
            }
            
            info.penetrationDepth = body.GetRadius() + minDist;
        }
        
        info.contactPoint = closestPoint;
    }
    
    return info;
}

void Collision::ResolveCollision(RigidBody& body1, RigidBody& body2, const CollisionInfo& collision) {
    if (!collision.hasCollision) return;
    
    // Separate the bodies first
    SeparateBodies(body1, body2, collision);
    
    // Calculate relative velocity
    Vector2D relativeVelocity = body2.GetVelocity() - body1.GetVelocity();
    float velocityAlongNormal = relativeVelocity.Dot(collision.normal);
    
    // Don't resolve if velocities are separating
    if (velocityAlongNormal > 0) return;
    
    // Calculate restitution (bounciness)
    float restitution = std::min(body1.GetRestitution(), body2.GetRestitution());
    
    // Calculate impulse scalar
    float impulseScalar = -(1 + restitution) * velocityAlongNormal;
    impulseScalar /= (body1.GetInverseMass() + body2.GetInverseMass());
    
    // Apply impulse
    Vector2D impulse = collision.normal * impulseScalar;
    
    if (!body1.IsStatic()) {
        body1.ApplyImpulse(impulse * -1);
    }
    if (!body2.IsStatic()) {
        body2.ApplyImpulse(impulse);
    }
    
    // Apply friction
    float frictionCoeff = std::sqrt(body1.GetFriction() * body2.GetFriction());
    ApplyFriction(body1, body2, collision.normal, frictionCoeff);
}

void Collision::ResolveStaticCollision(RigidBody& body, const CollisionInfo& collision) {
    if (!collision.hasCollision || body.IsStatic()) return;
    
    // Move the body out of the collision
    Vector2D correction = collision.normal * collision.penetrationDepth;
    body.SetPosition(body.GetPosition() + correction);
    
    // Calculate velocity along normal
    float velocityAlongNormal = body.GetVelocity().Dot(collision.normal);
    
    // Don't resolve if velocity is separating
    if (velocityAlongNormal > 0) return;
    
    // Apply restitution
    float restitution = body.GetRestitution();
    Vector2D newVelocity = body.GetVelocity() - collision.normal * (velocityAlongNormal * (1 + restitution));
    body.SetVelocity(newVelocity);
    
    // Apply friction
    Vector2D tangent = body.GetVelocity() - collision.normal * body.GetVelocity().Dot(collision.normal);
    if (tangent.LengthSq() > 0) {
        tangent.Normalize();
        float frictionImpulse = -body.GetVelocity().Dot(tangent) * body.GetFriction();
        body.SetVelocity(body.GetVelocity() + tangent * frictionImpulse);
    }
}

void Collision::SeparateBodies(RigidBody& body1, RigidBody& body2, const CollisionInfo& collision) {
    float totalInverseMass = body1.GetInverseMass() + body2.GetInverseMass();
    
    if (totalInverseMass <= 0) return; // Both bodies are static
    
    Vector2D correction = collision.normal * (collision.penetrationDepth / totalInverseMass);
    
    if (!body1.IsStatic()) {
        body1.SetPosition(body1.GetPosition() - correction * body1.GetInverseMass());
    }
    if (!body2.IsStatic()) {
        body2.SetPosition(body2.GetPosition() + correction * body2.GetInverseMass());
    }
}

void Collision::ApplyFriction(RigidBody& body1, RigidBody& body2, const Vector2D& normal, float frictionCoeff) {
    Vector2D relativeVelocity = body2.GetVelocity() - body1.GetVelocity();
    
    // Calculate tangent vector (perpendicular to normal)
    Vector2D tangent = relativeVelocity - normal * relativeVelocity.Dot(normal);
    
    if (tangent.LengthSq() > 0) {
        tangent.Normalize();
        
        // Calculate friction impulse
        float frictionImpulse = -relativeVelocity.Dot(tangent) * frictionCoeff;
        frictionImpulse /= (body1.GetInverseMass() + body2.GetInverseMass());
        
        Vector2D frictionForce = tangent * frictionImpulse;
        
        if (!body1.IsStatic()) {
            body1.ApplyImpulse(frictionForce * -1);
        }
        if (!body2.IsStatic()) {
            body2.ApplyImpulse(frictionForce);
        }
    }
}

Vector2D Collision::GetClosestPointOnLine(const Vector2D& point, const Vector2D& lineStart, const Vector2D& lineEnd) {
    Vector2D line = lineEnd - lineStart;
    float lineLength = line.LengthSq();
    
    if (lineLength == 0) {
        return lineStart; // Line is a point
    }
    
    float t = (point - lineStart).Dot(line) / lineLength;
    t = std::max(0.0f, std::min(1.0f, t)); // Clamp to line segment
    
    return lineStart + line * t;
}

Vector2D Collision::ClampVector(const Vector2D& vector, const Vector2D& min, const Vector2D& max) {
    return Vector2D(
        std::max(min.x, std::min(max.x, vector.x)),
        std::max(min.y, std::min(max.y, vector.y))
    );
}
