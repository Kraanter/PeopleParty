#ifndef MARBLE_MANIA_MARBLE_H
#define MARBLE_MANIA_MARBLE_H

#include "../../util/physics/physics_object.h"
#include "../../util/math/vector2d.h"
#include "../../client.h"
#include <string>

class MarbleManiaMarble {
private:
    PhysicsObject* m_physicsObject; // Owned by the physics world
    Client* m_owner;
    std::string m_id;
    
    // Game state
    bool m_hasFinished;
    float m_timeToFinish;
    int m_placement;
    bool m_isPlaced;
    float m_radius;
    
public:
    MarbleManiaMarble(PhysicsObject* physicsObject, Client* owner, float radius = 15.0f);
    ~MarbleManiaMarble() = default;
    
    // Basic properties
    const std::string& GetId() const { return m_id; }
    Client* GetOwner() const { return m_owner; }
    float GetRadius() const { return m_radius; }
    
    // Physics object access
    PhysicsObject* GetPhysicsObject() { return m_physicsObject; }
    const PhysicsObject* GetPhysicsObject() const { return m_physicsObject; }
    
    // Position and movement
    Vector2D GetPosition() const { return m_physicsObject->GetPosition(); }
    void SetPosition(const Vector2D& position) { m_physicsObject->SetPosition(position); }
    
    Vector2D GetVelocity() const { return m_physicsObject->GetVelocity(); }
    void SetVelocity(const Vector2D& velocity) { m_physicsObject->SetVelocity(velocity); }
    
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
    void SetRestitution(float restitution) { m_physicsObject->SetRestitution(restitution); }
    void SetFriction(float friction) { m_physicsObject->SetFriction(friction); }
    void SetMass(float mass) { m_physicsObject->SetMass(mass); }
    
    // Movement during placement phase
    void Move(const Vector2D& offset) { m_physicsObject->Move(offset); }
    
    // Utility
    void ResetToPosition(const Vector2D& position);
    void Stop();
};

#endif // MARBLE_MANIA_MARBLE_H
