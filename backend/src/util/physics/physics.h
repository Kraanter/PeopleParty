#ifndef PHYSICS_H
#define PHYSICS_H

// Main header file that includes all physics components
#include "collision_shape.h"
#include "physics_object.h"
#include "collision_detector.h"
#include "physics_world.h"

// Factory functions for easy object creation
namespace PhysicsFactory {
    
    // Create a dynamic circle object (falls under gravity)
    inline std::unique_ptr<PhysicsObject> CreateDynamicCircle(const Vector2D& position, float radius, 
                                                             float mass = 1.0f, float restitution = 0.6f, float friction = 0.1f) {
        auto shape = std::make_unique<CircleShape>(position, radius);
        auto object = std::make_unique<PhysicsObject>(std::move(shape), ObjectType::DYNAMIC, mass, restitution, friction);
        return object;
    }
    
    // Create a static circle object (doesn't move)
    inline std::unique_ptr<PhysicsObject> CreateStaticCircle(const Vector2D& position, float radius) {
        auto shape = std::make_unique<CircleShape>(position, radius);
        auto object = std::make_unique<PhysicsObject>(std::move(shape), ObjectType::STATIC);
        return object;
    }
    
    // Create a dynamic rectangle object (falls under gravity)
    inline std::unique_ptr<PhysicsObject> CreateDynamicRectangle(const Vector2D& position, float width, float height, 
                                                                float rotation = 0.0f, float mass = 1.0f, 
                                                                float restitution = 0.6f, float friction = 0.1f) {
        auto shape = std::make_unique<RectangleShape>(position, width, height, rotation);
        auto object = std::make_unique<PhysicsObject>(std::move(shape), ObjectType::DYNAMIC, mass, restitution, friction);
        return object;
    }
    
    // Create a static rectangle object (doesn't move)
    inline std::unique_ptr<PhysicsObject> CreateStaticRectangle(const Vector2D& position, float width, float height, 
                                                               float rotation = 0.0f) {
        auto shape = std::make_unique<RectangleShape>(position, width, height, rotation);
        auto object = std::make_unique<PhysicsObject>(std::move(shape), ObjectType::STATIC);
        return object;
    }
}

#endif // PHYSICS_H