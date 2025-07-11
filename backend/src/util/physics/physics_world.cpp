#include "physics_world.h"
#include <algorithm>
#include <cmath>

PhysicsWorld::PhysicsWorld() 
    : m_gravity(0, 9.81f)
    , m_damping(0.005f)
    , m_worldMin(-1000, -1000)
    , m_worldMax(1000, 1000)
    , m_enableBroadPhase(true)
{
}

PhysicsWorld::PhysicsWorld(const Vector2D& worldMin, const Vector2D& worldMax)
    : m_gravity(0, 9.81f)
    , m_damping(0.005f)
    , m_worldMin(worldMin)
    , m_worldMax(worldMax)
    , m_enableBroadPhase(true)
{
}

PhysicsWorld::~PhysicsWorld() {
    ClearBodies();
}

void PhysicsWorld::SetWorldBounds(const Vector2D& min, const Vector2D& max) {
    m_worldMin = min;
    m_worldMax = max;
}

RigidBody* PhysicsWorld::CreateBody(const Vector2D& position, float mass, float radius) {
    auto body = std::make_unique<RigidBody>(position, mass, radius);
    RigidBody* bodyPtr = body.get();
    m_bodies.push_back(std::move(body));
    return bodyPtr;
}

void PhysicsWorld::RemoveBody(RigidBody* body) {
    auto it = std::find_if(m_bodies.begin(), m_bodies.end(),
        [body](const std::unique_ptr<RigidBody>& ptr) {
            return ptr.get() == body;
        });
    
    if (it != m_bodies.end()) {
        m_bodies.erase(it);
    }
}

void PhysicsWorld::ClearBodies() {
    m_bodies.clear();
}

void PhysicsWorld::AddBoundary(const Vector2D& start, const Vector2D& end) {
    m_boundaries.emplace_back(start, end);
}

void PhysicsWorld::ClearBoundaries() {
    m_boundaries.clear();
}

void PhysicsWorld::Step(float deltaTime) {
    // Apply global forces
    ApplyGravityToAll();
    ApplyDampingToAll();
    
    // Integrate physics
    IntegrateAll(deltaTime);
    
    // Handle collisions
    DetectCollisions();
    ResolveCollisions();
    
    // Check world boundaries
    CheckWorldBoundaries();
}

void PhysicsWorld::ApplyGravityToAll() {
    for (auto& body : m_bodies) {
        if (body->IsActive() && !body->IsStatic()) {
            body->ApplyGravity(m_gravity);
        }
    }
}

void PhysicsWorld::ApplyDampingToAll() {
    for (auto& body : m_bodies) {
        if (body->IsActive() && !body->IsStatic()) {
            body->ApplyDamping(m_damping);
        }
    }
}

void PhysicsWorld::CheckWorldBoundaries() {
    for (auto& body : m_bodies) {
        if (!body->IsActive() || body->IsStatic()) continue;
        
        Vector2D pos = body->GetPosition();
        Vector2D vel = body->GetVelocity();
        float radius = body->GetRadius();
        bool collision = false;
        
        // Check left boundary
        if (pos.x - radius < m_worldMin.x) {
            pos.x = m_worldMin.x + radius;
            vel.x = -vel.x * body->GetRestitution();
            collision = true;
        }
        
        // Check right boundary
        if (pos.x + radius > m_worldMax.x) {
            pos.x = m_worldMax.x - radius;
            vel.x = -vel.x * body->GetRestitution();
            collision = true;
        }
        
        // Check top boundary
        if (pos.y - radius < m_worldMin.y) {
            pos.y = m_worldMin.y + radius;
            vel.y = -vel.y * body->GetRestitution();
            collision = true;
        }
        
        // Check bottom boundary
        if (pos.y + radius > m_worldMax.y) {
            pos.y = m_worldMax.y - radius;
            vel.y = -vel.y * body->GetRestitution();
            collision = true;
        }
        
        if (collision) {
            body->SetPosition(pos);
            body->SetVelocity(vel);
        }
    }
}

RigidBody* PhysicsWorld::GetBodyAt(const Vector2D& position) const {
    for (const auto& body : m_bodies) {
        if (!body->IsActive()) continue;
        
        float distance = body->GetPosition().Distance(position);
        if (distance <= body->GetRadius()) {
            return body.get();
        }
    }
    return nullptr;
}

std::vector<RigidBody*> PhysicsWorld::GetBodiesInRadius(const Vector2D& center, float radius) const {
    std::vector<RigidBody*> result;
    
    for (const auto& body : m_bodies) {
        if (!body->IsActive()) continue;
        
        float distance = body->GetPosition().Distance(center);
        if (distance <= radius + body->GetRadius()) {
            result.push_back(body.get());
        }
    }
    
    return result;
}

void PhysicsWorld::IntegrateAll(float deltaTime) {
    for (auto& body : m_bodies) {
        if (body->IsActive()) {
            body->Integrate(deltaTime);
        }
    }
}

void PhysicsWorld::DetectCollisions() {
    // Body-to-body collisions
    for (size_t i = 0; i < m_bodies.size(); ++i) {
        for (size_t j = i + 1; j < m_bodies.size(); ++j) {
            if (!m_bodies[i]->IsActive() || !m_bodies[j]->IsActive()) continue;
            
            if (m_enableBroadPhase && !BroadPhaseCheck(*m_bodies[i], *m_bodies[j])) {
                continue;
            }
            
            HandleBodyCollision(*m_bodies[i], *m_bodies[j]);
        }
    }
    
    // Body-to-boundary collisions
    for (auto& body : m_bodies) {
        if (!body->IsActive()) continue;
        
        for (const auto& boundary : m_boundaries) {
            HandleBoundaryCollision(*body, boundary);
        }
    }
}

void PhysicsWorld::ResolveCollisions() {
    // Collisions are resolved immediately in DetectCollisions for simplicity
    // In a more advanced system, you might want to collect all collisions first
    // and then resolve them to avoid order-dependent behavior
}

bool PhysicsWorld::BroadPhaseCheck(const RigidBody& body1, const RigidBody& body2) const {
    float totalRadius = body1.GetRadius() + body2.GetRadius();
    return body1.GetPosition().DistanceSq(body2.GetPosition()) <= (totalRadius * totalRadius);
}

void PhysicsWorld::HandleBodyCollision(RigidBody& body1, RigidBody& body2) {
    CollisionInfo collision = Collision::CheckCircleCircle(body1, body2);
    
    if (collision.hasCollision) {
        Collision::ResolveCollision(body1, body2, collision);
    }
}

void PhysicsWorld::HandleBoundaryCollision(RigidBody& body, const Boundary& boundary) {
    CollisionInfo collision = Collision::CheckCircleLine(body, boundary.start, boundary.end);
    
    if (collision.hasCollision) {
        Collision::ResolveStaticCollision(body, collision);
    }
}
