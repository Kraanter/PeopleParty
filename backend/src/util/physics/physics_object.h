#ifndef PHYSICS_OBJECT_H
#define PHYSICS_OBJECT_H

#include "collision_shape.h"
#include "../math/vector2d.h"
#include <memory>

enum class ObjectType {
    STATIC,     // Does not move, does not fall
    DYNAMIC     // Affected by gravity, can move and collide
};

// Physics object that contains a collision shape and physics properties
class PhysicsObject {
private:
    std::unique_ptr<CollisionShape> m_shape;
    ObjectType m_type;
    
    // Physics properties
    Vector2D m_velocity;
    Vector2D m_acceleration;
    float m_mass;
    float m_restitution; // Bounciness (0 = no bounce, 1 = perfect bounce)
    float m_friction;    // Surface friction
    
    // Limits and constants
    static constexpr float MAX_VELOCITY = 500.0f; // Max speed for air resistance
    static constexpr float GRAVITY = 981.0f;      // Pixels per second squared
    
public:
    PhysicsObject(std::unique_ptr<CollisionShape> shape, ObjectType type, 
                  float mass = 1.0f, float restitution = 0.6f, float friction = 0.1f);
    
    // Shape access
    const CollisionShape* GetShape() const { return m_shape.get(); }
    CollisionShape* GetShape() { return m_shape.get(); }
    
    // Type and properties
    ObjectType GetType() const { return m_type; }
    void SetType(ObjectType type) { m_type = type; }
    
    float GetMass() const { return m_mass; }
    void SetMass(float mass) { m_mass = mass; }
    
    float GetRestitution() const { return m_restitution; }
    void SetRestitution(float restitution) { m_restitution = restitution; }
    
    float GetFriction() const { return m_friction; }
    void SetFriction(float friction) { m_friction = friction; }
    
    // Physics state
    const Vector2D& GetVelocity() const { return m_velocity; }
    void SetVelocity(const Vector2D& velocity) { m_velocity = velocity; }
    void AddVelocity(const Vector2D& deltaVelocity) { m_velocity += deltaVelocity; }
    
    const Vector2D& GetAcceleration() const { return m_acceleration; }
    void SetAcceleration(const Vector2D& acceleration) { m_acceleration = acceleration; }
    void AddAcceleration(const Vector2D& deltaAcceleration) { m_acceleration += deltaAcceleration; }
    
    // Position convenience methods
    Vector2D GetPosition() const { return m_shape->GetPosition(); }
    void SetPosition(const Vector2D& position) { m_shape->SetPosition(position); }
    void Move(const Vector2D& offset) { m_shape->Move(offset); }
    
    // Rotation convenience methods
    float GetRotation() const { return m_shape->GetRotation(); }
    void SetRotation(float rotation) { m_shape->SetRotation(rotation); }
    void Rotate(float deltaRotation) { m_shape->Rotate(deltaRotation); }
    
    // Physics update
    void Update(float deltaTime);
    
    // Force application
    void ApplyForce(const Vector2D& force);
    void ApplyImpulse(const Vector2D& impulse);
    
    // Collision response
    void HandleCollision(const PhysicsObject& other, const Vector2D& collisionNormal, 
                        float penetrationDepth, const Vector2D& collisionPoint);
    
private:
    void ApplyGravity(float deltaTime);
    void IntegrateVelocity(float deltaTime);
    void ApplyAirResistance(float deltaTime);
    void LimitVelocity();
};

#endif // PHYSICS_OBJECT_H