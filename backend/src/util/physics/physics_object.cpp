#include "physics_object.h"
#include <algorithm>
#include <cmath>

PhysicsObject::PhysicsObject(std::unique_ptr<CollisionShape> shape, ObjectType type, 
                            float mass, float restitution, float friction)
    : m_shape(std::move(shape)), m_type(type), m_mass(mass), 
      m_restitution(restitution), m_friction(friction) {
    m_velocity = Vector2D(0, 0);
    m_acceleration = Vector2D(0, 0);
}

void PhysicsObject::Update(float deltaTime) {
    if (m_type != ObjectType::DYNAMIC) {
        return; // Static objects don't move
    }
    
    // Apply gravity
    ApplyGravity(deltaTime);
    
    // Apply air resistance
    ApplyAirResistance(deltaTime);
    
    // Integrate velocity
    IntegrateVelocity(deltaTime);
    
    // Limit velocity for air resistance simulation
    LimitVelocity();
    
    // Update position based on velocity
    Vector2D displacement = m_velocity * deltaTime;
    m_shape->Move(displacement);
    
    // Reset acceleration for next frame
    m_acceleration = Vector2D(0, 0);
}

void PhysicsObject::ApplyForce(const Vector2D& force) {
    if (m_type != ObjectType::DYNAMIC || m_mass <= 0) {
        return;
    }
    
    // F = ma, so a = F/m
    Vector2D acceleration = force / m_mass;
    AddAcceleration(acceleration);
}

void PhysicsObject::ApplyImpulse(const Vector2D& impulse) {
    if (m_type != ObjectType::DYNAMIC || m_mass <= 0) {
        return;
    }
    
    // Impulse directly changes velocity: J = m * Δv, so Δv = J/m
    Vector2D deltaVelocity = impulse / m_mass;
    AddVelocity(deltaVelocity);
}

void PhysicsObject::HandleCollision(const PhysicsObject& other, const Vector2D& collisionNormal, 
                                   float penetrationDepth, const Vector2D& collisionPoint) {
    if (m_type != ObjectType::DYNAMIC) {
        return; // Static objects don't respond to collisions
    }
    
    // Move object out of penetration
    Vector2D separation = collisionNormal * penetrationDepth;
    m_shape->Move(separation);
    
    // Calculate relative velocity
    Vector2D relativeVelocity = m_velocity - other.GetVelocity();
    
    // Calculate relative velocity in collision normal direction
    float velocityAlongNormal = relativeVelocity.Dot(collisionNormal);
    
    // Do not resolve if velocities are separating
    if (velocityAlongNormal > 0) {
        return;
    }
    
    // Calculate restitution (bounciness)
    float combinedRestitution = std::min(m_restitution, other.GetRestitution());
    
    // Calculate impulse scalar
    float impulseScalar = -(1 + combinedRestitution) * velocityAlongNormal;
    
    // Apply mass weighting if other object is also dynamic
    if (other.GetType() == ObjectType::DYNAMIC) {
        impulseScalar /= (1.0f / m_mass + 1.0f / other.GetMass());
    } else {
        impulseScalar /= (1.0f / m_mass); // Other object has infinite mass
    }
    
    // Apply impulse
    Vector2D impulse = collisionNormal * impulseScalar;
    ApplyImpulse(impulse);
    
    // Apply friction
    Vector2D tangent = relativeVelocity - collisionNormal * velocityAlongNormal;
    if (tangent.Length() > 0.001f) {
        tangent = tangent.Normalized();
        
        float frictionMagnitude = std::abs(impulseScalar) * std::max(m_friction, other.GetFriction());
        Vector2D frictionImpulse = tangent * -frictionMagnitude;
        
        // Limit friction to not reverse velocity
        if (frictionImpulse.Length() > std::abs(velocityAlongNormal)) {
            frictionImpulse = tangent * -std::abs(velocityAlongNormal);
        }
        
        ApplyImpulse(frictionImpulse);
    }
}

void PhysicsObject::ApplyGravity(float deltaTime) {
    Vector2D gravity(0, GRAVITY);
    ApplyForce(gravity * m_mass);
}

void PhysicsObject::IntegrateVelocity(float deltaTime) {
    // Integrate acceleration into velocity
    m_velocity += m_acceleration * deltaTime;
}

void PhysicsObject::ApplyAirResistance(float deltaTime) {
    // Apply simple air resistance proportional to velocity squared
    float velocityMagnitude = m_velocity.Length();
    if (velocityMagnitude > 0.001f) {
        float airResistance = 0.01f * velocityMagnitude * velocityMagnitude;
        Vector2D resistanceForce = m_velocity.Normalized() * -airResistance;
        ApplyForce(resistanceForce);
    }
}

void PhysicsObject::LimitVelocity() {
    float speed = m_velocity.Length();
    if (speed > MAX_VELOCITY) {
        m_velocity = m_velocity.Normalized() * MAX_VELOCITY;
    }
}