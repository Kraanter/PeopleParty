#include "collision_detector.h"
#include <cmath>
#include <algorithm>
#include <limits>

CollisionInfo CollisionDetector::DetectCollision(const PhysicsObject& objectA, const PhysicsObject& objectB) {
    return DetectCollision(*objectA.GetShape(), *objectB.GetShape());
}

CollisionInfo CollisionDetector::DetectCollision(const CollisionShape& shapeA, const CollisionShape& shapeB) {
    ShapeType typeA = shapeA.GetType();
    ShapeType typeB = shapeB.GetType();
    
    if (typeA == ShapeType::CIRCLE && typeB == ShapeType::CIRCLE) {
        return CircleVsCircle(static_cast<const CircleShape&>(shapeA), 
                             static_cast<const CircleShape&>(shapeB));
    }
    else if (typeA == ShapeType::CIRCLE && typeB == ShapeType::RECTANGLE) {
        return CircleVsRectangle(static_cast<const CircleShape&>(shapeA), 
                                static_cast<const RectangleShape&>(shapeB));
    }
    else if (typeA == ShapeType::RECTANGLE && typeB == ShapeType::CIRCLE) {
        return RectangleVsCircle(static_cast<const RectangleShape&>(shapeA), 
                                static_cast<const CircleShape&>(shapeB));
    }
    else if (typeA == ShapeType::RECTANGLE && typeB == ShapeType::RECTANGLE) {
        return RectangleVsRectangle(static_cast<const RectangleShape&>(shapeA), 
                                   static_cast<const RectangleShape&>(shapeB));
    }
    
    return CollisionInfo(); // No collision
}

CollisionInfo CollisionDetector::CircleVsCircle(const CircleShape& circleA, const CircleShape& circleB) {
    Vector2D centerA = circleA.GetPosition();
    Vector2D centerB = circleB.GetPosition();
    float radiusA = circleA.GetRadius();
    float radiusB = circleB.GetRadius();
    
    Vector2D distance = centerB - centerA;
    float distanceLength = distance.Length();
    float combinedRadius = radiusA + radiusB;
    
    if (distanceLength >= combinedRadius) {
        return CollisionInfo(); // No collision
    }
    
    // Handle case where circles are at the same position
    if (distanceLength < 0.001f) {
        Vector2D normal(1.0f, 0.0f); // Default normal
        float penetrationDepth = combinedRadius;
        Vector2D contactPoint = centerA;
        return CollisionInfo(normal, penetrationDepth, contactPoint);
    }
    
    Vector2D normal = distance / distanceLength;
    float penetrationDepth = combinedRadius - distanceLength;
    Vector2D contactPoint = centerA + normal * radiusA;
    
    return CollisionInfo(normal, penetrationDepth, contactPoint);
}

CollisionInfo CollisionDetector::CircleVsRectangle(const CircleShape& circle, const RectangleShape& rectangle) {
    Vector2D circleCenter = circle.GetPosition();
    float circleRadius = circle.GetRadius();
    
    // Find the closest point on the rectangle to the circle center
    Vector2D closestPoint = rectangle.GetClosestPointToCircle(circleCenter);
    
    // Calculate distance from circle center to closest point
    Vector2D distance = circleCenter - closestPoint;
    float distanceLength = distance.Length();
    
    if (distanceLength >= circleRadius) {
        return CollisionInfo(); // No collision
    }
    
    // Handle case where circle center is inside rectangle
    if (distanceLength < 0.001f) {
        // Find the shortest distance to any edge
        Vector2D corners[4];
        rectangle.GetAllCorners(corners);
        
        float minDistance = std::numeric_limits<float>::max();
        Vector2D bestNormal;
        
        for (int i = 0; i < 4; i++) {
            Vector2D edgeStart = corners[i];
            Vector2D edgeEnd = corners[(i + 1) % 4];
            float distance = DistancePointToLine(circleCenter, edgeStart, edgeEnd);
            
            if (distance < minDistance) {
                minDistance = distance;
                Vector2D edgeVector = edgeEnd - edgeStart;
                bestNormal = Vector2D(-edgeVector.y, edgeVector.x);
                bestNormal.Normalize();
            }
        }
        
        float penetrationDepth = circleRadius + minDistance;
        return CollisionInfo(bestNormal, penetrationDepth, closestPoint);
    }
    
    Vector2D normal = distance;
    normal.Normalize();
    float penetrationDepth = circleRadius - distanceLength;
    
    return CollisionInfo(normal, penetrationDepth, closestPoint);
}

CollisionInfo CollisionDetector::RectangleVsCircle(const RectangleShape& rectangle, const CircleShape& circle) {
    CollisionInfo info = CircleVsRectangle(circle, rectangle);
    if (info.hasCollision) {
        info.normal = info.normal * -1.0f; // Flip normal since we swapped the order
    }
    return info;
}

CollisionInfo CollisionDetector::RectangleVsRectangle(const RectangleShape& rectA, const RectangleShape& rectB) {
    CollisionInfo info;
    if (SATCollision(rectA, rectB, info)) {
        return info;
    }
    return CollisionInfo(); // No collision
}

bool CollisionDetector::SATCollision(const RectangleShape& rectA, const RectangleShape& rectB, CollisionInfo& info) {
    float minOverlap = std::numeric_limits<float>::max();
    Vector2D separationAxis;
    
    // Test axes from both rectangles
    for (int rect = 0; rect < 2; rect++) {
        const RectangleShape& currentRect = (rect == 0) ? rectA : rectB;
        
        for (int axis = 0; axis < 2; axis++) {
            Vector2D axisVector = GetRectangleAxis(currentRect, axis);
            
            float minA, maxA, minB, maxB;
            ProjectRectangleOntoAxis(rectA, axisVector, minA, maxA);
            ProjectRectangleOntoAxis(rectB, axisVector, minB, maxB);
            
            // Check for separation
            if (maxA < minB || maxB < minA) {
                return false; // Separating axis found, no collision
            }
            
            // Calculate overlap
            float overlap = std::min(maxA - minB, maxB - minA);
            if (overlap < minOverlap) {
                minOverlap = overlap;
                separationAxis = axisVector;
                
                // Ensure axis points from A to B
                Vector2D centerDistance = rectB.GetPosition() - rectA.GetPosition();
                if (centerDistance.Dot(axisVector) < 0) {
                    separationAxis = separationAxis * -1.0f;
                }
            }
        }
    }
    
    // If we get here, all axes overlap, so there is a collision
    info.hasCollision = true;
    info.normal = separationAxis;
    info.penetrationDepth = minOverlap;
    
    // Calculate contact point (approximation using centers)
    Vector2D centerA = rectA.GetPosition();
    Vector2D centerB = rectB.GetPosition();
    info.contactPoint = (centerA + centerB) * 0.5f;
    
    return true;
}

Vector2D CollisionDetector::GetRectangleAxis(const RectangleShape& rectangle, int axisIndex) {
    Vector2D corners[4];
    rectangle.GetAllCorners(corners);
    
    if (axisIndex == 0) {
        // First axis: edge from corner 0 to corner 1
        Vector2D edge = corners[1] - corners[0];
        Vector2D perpendicular = Vector2D(-edge.y, edge.x);
        perpendicular.Normalize();
        return perpendicular; // Perpendicular to edge
    } else {
        // Second axis: edge from corner 1 to corner 2
        Vector2D edge = corners[2] - corners[1];
        Vector2D perpendicular = Vector2D(-edge.y, edge.x);
        perpendicular.Normalize();
        return perpendicular; // Perpendicular to edge
    }
}

void CollisionDetector::ProjectRectangleOntoAxis(const RectangleShape& rectangle, const Vector2D& axis, float& min, float& max) {
    Vector2D corners[4];
    rectangle.GetAllCorners(corners);
    
    min = max = corners[0].Dot(axis);
    
    for (int i = 1; i < 4; i++) {
        float projection = corners[i].Dot(axis);
        min = std::min(min, projection);
        max = std::max(max, projection);
    }
}

float CollisionDetector::DistancePointToLine(const Vector2D& point, const Vector2D& lineStart, const Vector2D& lineEnd) {
    Vector2D projected = ProjectPointOntoLine(point, lineStart, lineEnd);
    return (point - projected).Length();
}

Vector2D CollisionDetector::ProjectPointOntoLine(const Vector2D& point, const Vector2D& lineStart, const Vector2D& lineEnd) {
    Vector2D lineVector = lineEnd - lineStart;
    Vector2D pointVector = point - lineStart;
    
    float lineLength = lineVector.Length();
    if (lineLength < 0.001f) {
        return lineStart; // Line is too short
    }
    
    float projection = pointVector.Dot(lineVector) / (lineLength * lineLength);
    projection = std::max(0.0f, std::min(1.0f, projection)); // Clamp to line segment
    
    return lineStart + lineVector * projection;
}