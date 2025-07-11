#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include "../math/vector2d.h"

class RigidBody {
private:
    Vector2D m_position;
    Vector2D m_velocity;
    Vector2D m_acceleration;
    Vector2D m_force;
    
    float m_mass;
    float m_inverseMass;
    float m_restitution;  // bounciness (0 = no bounce, 1 = perfect bounce)
    float m_friction;     // friction coefficient
    float m_radius;       // for circular bodies like marbles
    
    bool m_isStatic;      // static bodies don't move
    bool m_isActive;      // inactive bodies are not simulated
    
public:
    RigidBody();
    RigidBody(Vector2D position, float mass, float radius);
    
    // Position and movement
    void SetPosition(const Vector2D& position) { m_position = position; }
    const Vector2D& GetPosition() const { return m_position; }
    
    void SetVelocity(const Vector2D& velocity) { m_velocity = velocity; }
    const Vector2D& GetVelocity() const { return m_velocity; }
    
    void SetAcceleration(const Vector2D& acceleration) { m_acceleration = acceleration; }
    const Vector2D& GetAcceleration() const { return m_acceleration; }
    
    // Forces
    void AddForce(const Vector2D& force) { m_force += force; }
    void ClearForces() { m_force.Zero(); }
    const Vector2D& GetForce() const { return m_force; }
    
    // Mass properties
    void SetMass(float mass);
    float GetMass() const { return m_mass; }
    float GetInverseMass() const { return m_inverseMass; }
    
    // Material properties
    void SetRestitution(float restitution) { m_restitution = restitution; }
    float GetRestitution() const { return m_restitution; }
    
    void SetFriction(float friction) { m_friction = friction; }
    float GetFriction() const { return m_friction; }
    
    void SetRadius(float radius) { m_radius = radius; }
    float GetRadius() const { return m_radius; }
    
    // State
    void SetStatic(bool isStatic) { m_isStatic = isStatic; }
    bool IsStatic() const { return m_isStatic; }
    
    void SetActive(bool isActive) { m_isActive = isActive; }
    bool IsActive() const { return m_isActive; }
    
    // Physics integration
    void Integrate(float deltaTime);
    
    // Collision detection
    bool CheckCollision(const RigidBody& other) const;
    float GetDistanceToBody(const RigidBody& other) const;
    
    // Utility methods
    void ApplyImpulse(const Vector2D& impulse);
    void ApplyGravity(const Vector2D& gravity);
    void ApplyDamping(float damping);
};

#endif // RIGID_BODY_H
