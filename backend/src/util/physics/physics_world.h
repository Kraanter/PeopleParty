#ifndef PHYSICS_WORLD_H
#define PHYSICS_WORLD_H

#include "physics_object.h"
#include "collision_detector.h"
#include <vector>
#include <memory>

// Main physics simulation class
class PhysicsWorld {
private:
    std::vector<std::unique_ptr<PhysicsObject>> m_objects;
    Vector2D m_worldMin;
    Vector2D m_worldMax;
    bool m_hasBoundaries;
    
    // World boundaries (static rectangle objects)
    std::vector<PhysicsObject*> m_boundaries;
    
public:
    PhysicsWorld(const Vector2D& worldMin = Vector2D(-1000, -1000), 
                 const Vector2D& worldMax = Vector2D(1000, 1000));
    ~PhysicsWorld();
    
    // Object management
    PhysicsObject* AddObject(std::unique_ptr<PhysicsObject> object);
    void RemoveObject(PhysicsObject* object);
    void Clear();
    
    // World boundaries
    void SetWorldBounds(const Vector2D& worldMin, const Vector2D& worldMax);
    void EnableBoundaries(bool enable);
    
    // Simulation
    void Step(float deltaTime);
    
    // Access
    const std::vector<std::unique_ptr<PhysicsObject>>& GetObjects() const { return m_objects; }
    size_t GetObjectCount() const { return m_objects.size(); }
    
    // Utility
    std::vector<PhysicsObject*> GetObjectsInRadius(const Vector2D& center, float radius) const;
    PhysicsObject* GetObjectAtPoint(const Vector2D& point) const;
    
private:
    void UpdateObjects(float deltaTime);
    void DetectAndResolveCollisions();
    void CheckBoundaryCollisions();
    void CreateBoundaries();
    void DestroyBoundaries();
    
    bool IsPointInShape(const Vector2D& point, const CollisionShape& shape) const;
};

#endif // PHYSICS_WORLD_H