#ifndef PHYSICS_CONSTANTS_H
#define PHYSICS_CONSTANTS_H

#include "../math/vector2d.h"

namespace Physics {
    // Common physics constants
    const float DEFAULT_GRAVITY = 9.81f;
    const float DEFAULT_DAMPING = 0.005f;
    const float DEFAULT_RESTITUTION = 0.7f;
    const float DEFAULT_FRICTION = 0.3f;
    
    // Marble-specific constants
    const float MARBLE_DEFAULT_RADIUS = 15.0f;
    const float MARBLE_DEFAULT_MASS = 1.0f;
    const float MARBLE_MIN_VELOCITY = 0.1f;
    const float MARBLE_MAX_VELOCITY = 1000.0f;
    
    // Collision constants
    const float COLLISION_THRESHOLD = 0.001f;
    const float SEPARATION_BIAS = 0.8f;
    const float PENETRATION_ALLOWANCE = 0.01f;
    
    // Physics simulation
    const float MIN_TIMESTEP = 0.0001f;
    const float MAX_TIMESTEP = 0.0333f; // ~30 FPS
    const int MAX_COLLISION_ITERATIONS = 10;
    
    // Utility functions
    inline float Clamp(float value, float min, float max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }
    
    inline Vector2D ClampVector(const Vector2D& vector, float maxLength) {
        float length = vector.Length();
        if (length > maxLength) {
            return vector * (maxLength / length);
        }
        return vector;
    }
    
    inline float RandomFloat(float min, float max) {
        return min + (max - min) * (rand() / (float)RAND_MAX);
    }
    
    inline Vector2D RandomVector2D(float minX, float maxX, float minY, float maxY) {
        return Vector2D(RandomFloat(minX, maxX), RandomFloat(minY, maxY));
    }
    
    inline bool IsNearZero(float value, float threshold = 0.001f) {
        return abs(value) < threshold;
    }
    
    inline bool IsNearZero(const Vector2D& vector, float threshold = 0.001f) {
        return vector.LengthSq() < (threshold * threshold);
    }
    
    // Physics conversion utilities
    inline float DegreesToRadians(float degrees) {
        return degrees * M_PI / 180.0f;
    }
    
    inline float RadiansToDegrees(float radians) {
        return radians * 180.0f / M_PI;
    }
    
    // Force generators
    inline Vector2D GravityForce(float mass, const Vector2D& gravity = Vector2D(0, DEFAULT_GRAVITY)) {
        return gravity * mass;
    }
    
    inline Vector2D DragForce(const Vector2D& velocity, float dragCoeff) {
        float speed = velocity.Length();
        if (speed > 0) {
            return velocity * (-dragCoeff * speed);
        }
        return Vector2D(0, 0);
    }
    
    inline Vector2D SpringForce(const Vector2D& position, const Vector2D& anchor, float springConstant, float restLength = 0) {
        Vector2D displacement = position - anchor;
        float distance = displacement.Length();
        
        if (distance > 0) {
            float force = springConstant * (distance - restLength);
            return displacement * (-force / distance);
        }
        return Vector2D(0, 0);
    }
    
    // Collision response utilities
    inline float CalculateRestitution(float e1, float e2) {
        return (e1 + e2) * 0.5f; // Average restitution
    }
    
    inline float CalculateFriction(float f1, float f2) {
        return sqrt(f1 * f2); // Geometric mean
    }
    
    // Physics debugging
    struct PhysicsDebugInfo {
        int totalBodies;
        int activeBodies;
        int collisions;
        float simulationTime;
        float averageVelocity;
        float totalKineticEnergy;
        
        PhysicsDebugInfo() : totalBodies(0), activeBodies(0), collisions(0), 
                            simulationTime(0), averageVelocity(0), totalKineticEnergy(0) {}
    };
}

#endif // PHYSICS_CONSTANTS_H
