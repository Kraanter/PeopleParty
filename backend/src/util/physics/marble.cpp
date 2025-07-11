#include "marble.h"
#include <cmath>
#include <random>

MarbleBase::MarbleBase(const Vector2D& position, MarbleType type)
    : RigidBody(position, 1.0f, 15.0f)
    , m_type(type)
    , m_color(GenerateRandomColor())
    , m_baseRadius(15.0f)
    , m_health(100.0f)
    , m_isGlowing(false)
    , m_glowIntensity(0.0f)
    , m_stickyTime(0.0f)
    , m_bounceMultiplier(1.0f)
    , m_rotationAngle(0.0f)
    , m_rotationSpeed(0.0f)
    , m_pointValue(10)
{
    InitializeTypeProperties();
}

void MarbleBase::SetType(MarbleType type) {
    m_type = type;
    InitializeTypeProperties();
}

void MarbleBase::ApplySticky(float duration) {
    m_stickyTime = duration;
}

void MarbleBase::TakeDamage(float damage) {
    m_health -= damage;
    if (m_health < 0) {
        m_health = 0;
    }
}

void MarbleBase::Update(float deltaTime) {
    UpdateRotation(deltaTime);
    UpdateSpecialEffects(deltaTime);
}

void MarbleBase::OnCollision(const MarbleBase& other) {
    // Special collision behavior based on marble type
    switch (m_type) {
        case MarbleType::BOUNCY:
            // Bouncy marbles transfer more energy
            break;
        case MarbleType::STICKY:
            // Sticky marbles might stick together
            if (m_stickyTime > 0) {
                // Reduce relative velocity
                Vector2D relativeVel = GetVelocity() - other.GetVelocity();
                SetVelocity(GetVelocity() - relativeVel * 0.5f);
            }
            break;
        case MarbleType::HEAVY:
            // Heavy marbles are less affected by collisions
            break;
        default:
            break;
    }
}

void MarbleBase::OnBoundaryCollision(const Vector2D& normal) {
    // Apply bounce multiplier for bouncy marbles
    if (m_type == MarbleType::BOUNCY) {
        Vector2D vel = GetVelocity();
        float velocityAlongNormal = vel.Dot(normal);
        
        if (velocityAlongNormal < 0) {
            Vector2D bounceVel = vel - normal * (velocityAlongNormal * 2.0f * m_bounceMultiplier);
            SetVelocity(bounceVel);
        }
    }
}

MarbleBase* MarbleBase::CreateNormalMarble(const Vector2D& position) {
    return new MarbleBase(position, MarbleType::NORMAL);
}

MarbleBase* MarbleBase::CreateHeavyMarble(const Vector2D& position) {
    return new MarbleBase(position, MarbleType::HEAVY);
}

MarbleBase* MarbleBase::CreateLightMarble(const Vector2D& position) {
    return new MarbleBase(position, MarbleType::LIGHT);
}

MarbleBase* MarbleBase::CreateBouncyMarble(const Vector2D& position) {
    return new MarbleBase(position, MarbleType::BOUNCY);
}

MarbleBase* MarbleBase::CreateStickyMarble(const Vector2D& position) {
    return new MarbleBase(position, MarbleType::STICKY);
}

void MarbleBase::InitializeTypeProperties() {
    switch (m_type) {
        case MarbleType::NORMAL:
            SetMass(1.0f);
            SetRadius(m_baseRadius);
            SetRestitution(0.7f);
            SetFriction(0.3f);
            m_pointValue = 10;
            m_rotationSpeed = 1.0f;
            break;
            
        case MarbleType::HEAVY:
            SetMass(3.0f);
            SetRadius(m_baseRadius * 1.2f);
            SetRestitution(0.5f);
            SetFriction(0.5f);
            m_pointValue = 25;
            m_rotationSpeed = 0.5f;
            m_color = 0x808080; // Gray
            break;
            
        case MarbleType::LIGHT:
            SetMass(0.5f);
            SetRadius(m_baseRadius * 0.8f);
            SetRestitution(0.9f);
            SetFriction(0.1f);
            m_pointValue = 5;
            m_rotationSpeed = 2.0f;
            m_color = 0xFFFFFF; // White
            break;
            
        case MarbleType::BOUNCY:
            SetMass(1.0f);
            SetRadius(m_baseRadius);
            SetRestitution(1.2f);
            SetFriction(0.2f);
            m_bounceMultiplier = 1.5f;
            m_pointValue = 20;
            m_rotationSpeed = 1.5f;
            m_color = 0x00FF00; // Green
            m_isGlowing = true;
            m_glowIntensity = 0.8f;
            break;
            
        case MarbleType::STICKY:
            SetMass(1.5f);
            SetRadius(m_baseRadius * 1.1f);
            SetRestitution(0.3f);
            SetFriction(0.8f);
            m_pointValue = 15;
            m_rotationSpeed = 0.8f;
            m_color = 0xFF8000; // Orange
            break;
    }
}

void MarbleBase::UpdateRotation(float deltaTime) {
    // Update rotation based on velocity
    float velocityMagnitude = GetVelocity().Length();
    m_rotationAngle += velocityMagnitude * m_rotationSpeed * deltaTime;
    
    // Keep angle in reasonable range
    while (m_rotationAngle > 2 * M_PI) {
        m_rotationAngle -= 2 * M_PI;
    }
}

void MarbleBase::UpdateSpecialEffects(float deltaTime) {
    // Update sticky time
    if (m_stickyTime > 0) {
        m_stickyTime -= deltaTime;
        if (m_stickyTime < 0) {
            m_stickyTime = 0;
        }
    }
    
    // Update glow effect for bouncy marbles
    if (m_type == MarbleType::BOUNCY && m_isGlowing) {
        m_glowIntensity = 0.5f + 0.3f * sin(m_rotationAngle * 3.0f);
    }
}

uint32_t MarbleBase::GenerateRandomColor() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint32_t> colorDist(0x000000, 0xFFFFFF);
    
    return colorDist(gen) | 0xFF000000; // Ensure alpha is 255
}
