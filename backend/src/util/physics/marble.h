#ifndef MARBLE_BASE_H
#define MARBLE_BASE_H

#include "rigid_body.h"
#include "../math/vector2d.h"
#include <cstdint>

enum class MarbleType {
    NORMAL,
    HEAVY,
    LIGHT,
    BOUNCY,
    STICKY
};

class MarbleBase : public RigidBody {
private:
    MarbleType m_type;
    uint32_t m_color;
    float m_baseRadius;
    float m_health;
    bool m_isGlowing;
    float m_glowIntensity;
    
    // Special marble properties
    float m_stickyTime;        // For sticky marbles
    float m_bounceMultiplier;  // For bouncy marbles
    
    // Visual effects
    float m_rotationAngle;
    float m_rotationSpeed;
    
    // Scoring
    int m_pointValue;
    
public:
    MarbleBase(const Vector2D& position, MarbleType type = MarbleType::NORMAL);
    
    // Type and properties
    void SetType(MarbleType type);
    MarbleType GetType() const { return m_type; }
    
    void SetColor(uint32_t color) { m_color = color; }
    uint32_t GetColor() const { return m_color; }
    
    void SetHealth(float health) { m_health = health; }
    float GetHealth() const { return m_health; }
    
    void SetPointValue(int points) { m_pointValue = points; }
    int GetPointValue() const { return m_pointValue; }
    
    // Visual properties
    void SetGlowing(bool glowing) { m_isGlowing = glowing; }
    bool IsGlowing() const { return m_isGlowing; }
    
    void SetGlowIntensity(float intensity) { m_glowIntensity = intensity; }
    float GetGlowIntensity() const { return m_glowIntensity; }
    
    float GetRotationAngle() const { return m_rotationAngle; }
    void SetRotationSpeed(float speed) { m_rotationSpeed = speed; }
    
    // Special abilities
    void ApplySticky(float duration);
    bool IsSticky() const { return m_stickyTime > 0; }
    
    void SetBounceMultiplier(float multiplier) { m_bounceMultiplier = multiplier; }
    float GetBounceMultiplier() const { return m_bounceMultiplier; }
    
    // Damage and destruction
    void TakeDamage(float damage);
    bool IsDestroyed() const { return m_health <= 0; }
    
    // Update method for marble-specific logic
    void Update(float deltaTime);
    
    // Collision response override
    void OnCollision(const MarbleBase& other);
    void OnBoundaryCollision(const Vector2D& normal);
    
    // Factory methods for different marble types
    static MarbleBase* CreateNormalMarble(const Vector2D& position);
    static MarbleBase* CreateHeavyMarble(const Vector2D& position);
    static MarbleBase* CreateLightMarble(const Vector2D& position);
    static MarbleBase* CreateBouncyMarble(const Vector2D& position);
    static MarbleBase* CreateStickyMarble(const Vector2D& position);
    
private:
    void InitializeTypeProperties();
    void UpdateRotation(float deltaTime);
    void UpdateSpecialEffects(float deltaTime);
    uint32_t GenerateRandomColor();
};

#endif // MARBLE_BASE_H
