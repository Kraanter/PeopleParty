#include "rigid_body.h"
#include <cmath>

RigidBody::RigidBody() 
    : m_position(0, 0)
    , m_velocity(0, 0)
    , m_acceleration(0, 0)
    , m_force(0, 0)
    , m_mass(1.0f)
    , m_inverseMass(1.0f)
    , m_restitution(0.8f)
    , m_friction(0.2f)
    , m_radius(10.0f)
    , m_isStatic(false)
    , m_isActive(true)
{
}

RigidBody::RigidBody(Vector2D position, float mass, float radius)
    : m_position(position)
    , m_velocity(0, 0)
    , m_acceleration(0, 0)
    , m_force(0, 0)
    , m_mass(mass)
    , m_restitution(0.8f)
    , m_friction(0.2f)
    , m_radius(radius)
    , m_isStatic(false)
    , m_isActive(true)
{
    if (mass > 0.0f) {
        m_inverseMass = 1.0f / mass;
    } else {
        m_inverseMass = 0.0f;
        m_isStatic = true;
    }
}

void RigidBody::SetMass(float mass) {
    m_mass = mass;
    if (mass > 0.0f) {
        m_inverseMass = 1.0f / mass;
        m_isStatic = false;
    } else {
        m_inverseMass = 0.0f;
        m_isStatic = true;
    }
}

void RigidBody::Integrate(float deltaTime) {
    if (!m_isActive || m_isStatic) {
        return;
    }
    
    // Calculate acceleration from force: F = ma -> a = F/m
    if (m_inverseMass > 0.0f) {
        m_acceleration = m_force * m_inverseMass;
    }
    
    // Update velocity: v = v0 + a*dt
    m_velocity += m_acceleration * deltaTime;
    
    // Update position: x = x0 + v*dt
    m_position += m_velocity * deltaTime;
    
    // Clear forces for next frame
    ClearForces();
}

bool RigidBody::CheckCollision(const RigidBody& other) const {
    if (!m_isActive || !other.m_isActive) {
        return false;
    }
    
    float distance = GetDistanceToBody(other);
    float minDistance = m_radius + other.m_radius;
    
    return distance <= minDistance;
}

float RigidBody::GetDistanceToBody(const RigidBody& other) const {
    return m_position.Distance(other.m_position);
}

void RigidBody::ApplyImpulse(const Vector2D& impulse) {
    if (!m_isActive || m_isStatic) {
        return;
    }
    
    // Apply impulse: v = v0 + J/m
    m_velocity += impulse * m_inverseMass;
}

void RigidBody::ApplyGravity(const Vector2D& gravity) {
    if (!m_isActive || m_isStatic) {
        return;
    }
    
    // Apply gravity force: F = mg
    AddForce(gravity * m_mass);
}

void RigidBody::ApplyDamping(float damping) {
    if (!m_isActive || m_isStatic) {
        return;
    }
    
    // Apply velocity damping
    m_velocity *= (1.0f - damping);
}
