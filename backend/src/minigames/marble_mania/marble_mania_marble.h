#ifndef MARBLE_MANIA_MARBLE_H
#define MARBLE_MANIA_MARBLE_H

#include "../../util/math/vector2d.h"
#include "../../client.h"
#include <box2d/box2d.h>
#include <string>

class MarbleManiaMarble {
private:
    b2Body* m_body; // Box2D body (owned by the world)
    Client* m_owner;
    std::string m_id;
    
    // Game state
    bool m_hasFinished;
    float m_timeToFinish;
    int m_placement;
    bool m_isPlaced;
    float m_radius;
    
public:
    MarbleManiaMarble(b2Body* body, Client* owner, float radius = 15.0f);
    ~MarbleManiaMarble() = default;
    
    // Basic properties
    const std::string& GetId() const { return m_id; }
    Client* GetOwner() const { return m_owner; }
    float GetRadius() const { return m_radius * 100.0f; } // Convert from meters to pixels
    
    // Physics object access
    b2Body* GetBody() { return m_body; }
    const b2Body* GetBody() const { return m_body; }
    
    // Position and movement
    Vector2D GetPosition() const { 
        b2Vec2 pos = m_body->GetPosition();
        return Vector2D(pos.x * 100.0f, pos.y * 100.0f); // Convert from meters to pixels
    }
    void SetPosition(const Vector2D& position) { 
        m_body->SetTransform(b2Vec2(position.x / 100.0f, position.y / 100.0f), m_body->GetAngle());
    }
    
    Vector2D GetVelocity() const { 
        b2Vec2 vel = m_body->GetLinearVelocity();
        return Vector2D(vel.x * 100.0f, vel.y * 100.0f); // Convert from m/s to pixels/s
    }
    void SetVelocity(const Vector2D& velocity) { 
        m_body->SetLinearVelocity(b2Vec2(velocity.x / 100.0f, velocity.y / 100.0f));
    }
    
    // Game state
    bool HasFinished() const { return m_hasFinished; }
    void SetFinished(bool finished, float gameTime) { 
        m_hasFinished = finished; 
        if (finished && m_timeToFinish == 0.0f) {
            m_timeToFinish = gameTime;
        }
    }
    
    float GetTimeToFinish() const { return m_timeToFinish; }
    
    int GetPlacement() const { return m_placement; }
    void SetPlacement(int placement) { m_placement = placement; }
    
    bool IsPlaced() const { return m_isPlaced; }
    void SetPlaced(bool placed) { m_isPlaced = placed; }
    
    // Physics properties
    void SetRestitution(float restitution);
    void SetFriction(float friction);
    void SetDensity(float density);
    
    // Movement during placement phase
    void Move(const Vector2D& offset) { 
        Vector2D currentPos = GetPosition();
        SetPosition(currentPos + offset);
    }
    
    // Utility
    void ResetToPosition(const Vector2D& position);
    void Stop();
};

#endif // MARBLE_MANIA_MARBLE_H
