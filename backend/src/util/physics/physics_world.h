#ifndef PHYSICS_WORLD_H
#define PHYSICS_WORLD_H

#include <vector>
#include <memory>
#include "../math/vector2d.h"
#include "rigid_body.h"
#include "collision.h"

struct Boundary {
    Vector2D start;
    Vector2D end;
    
    Boundary(const Vector2D& s, const Vector2D& e) : start(s), end(e) {}
};

class PhysicsWorld {
private:
    std::vector<std::unique_ptr<RigidBody>> m_bodies;
    std::vector<Boundary> m_boundaries;
    
    Vector2D m_gravity;
    float m_damping;
    Vector2D m_worldMin;
    Vector2D m_worldMax;
    
    // Collision detection optimization
    bool m_enableBroadPhase;
    
public:
    PhysicsWorld();
    PhysicsWorld(const Vector2D& worldMin, const Vector2D& worldMax);
    ~PhysicsWorld();
    
    // World properties
    void SetGravity(const Vector2D& gravity) { m_gravity = gravity; }
    const Vector2D& GetGravity() const { return m_gravity; }
    
    void SetDamping(float damping) { m_damping = damping; }
    float GetDamping() const { return m_damping; }
    
    void SetWorldBounds(const Vector2D& min, const Vector2D& max);
    const Vector2D& GetWorldMin() const { return m_worldMin; }
    const Vector2D& GetWorldMax() const { return m_worldMax; }
    
    // Body management
    RigidBody* CreateBody(const Vector2D& position, float mass, float radius);
    void RemoveBody(RigidBody* body);
    void ClearBodies();
    
    const std::vector<std::unique_ptr<RigidBody>>& GetBodies() const { return m_bodies; }
    
    // Boundary management
    void AddBoundary(const Vector2D& start, const Vector2D& end);
    void ClearBoundaries();
    const std::vector<Boundary>& GetBoundaries() const { return m_boundaries; }
    
    // Physics simulation
    void Step(float deltaTime);
    void SetBroadPhaseEnabled(bool enabled) { m_enableBroadPhase = enabled; }
    
    // Utility methods
    void ApplyGravityToAll();
    void ApplyDampingToAll();
    void CheckWorldBoundaries();
    
    // Query methods
    RigidBody* GetBodyAt(const Vector2D& position) const;
    std::vector<RigidBody*> GetBodiesInRadius(const Vector2D& center, float radius) const;
    
private:
    // Internal physics methods
    void IntegrateAll(float deltaTime);
    void DetectCollisions();
    void ResolveCollisions();
    
    // Collision detection helpers
    bool BroadPhaseCheck(const RigidBody& body1, const RigidBody& body2) const;
    void HandleBodyCollision(RigidBody& body1, RigidBody& body2);
    void HandleBoundaryCollision(RigidBody& body, const Boundary& boundary);
};

#endif // PHYSICS_WORLD_H
