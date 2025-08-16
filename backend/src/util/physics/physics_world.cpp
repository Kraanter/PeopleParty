#include "physics_world.h"
#include <algorithm>

PhysicsWorld::PhysicsWorld(const Vector2D& worldMin, const Vector2D& worldMax)
    : m_worldMin(worldMin), m_worldMax(worldMax), m_hasBoundaries(false) {
}

PhysicsWorld::~PhysicsWorld() {
    Clear();
}

PhysicsObject* PhysicsWorld::AddObject(std::unique_ptr<PhysicsObject> object) {
    PhysicsObject* ptr = object.get();
    m_objects.push_back(std::move(object));
    return ptr;
}

void PhysicsWorld::RemoveObject(PhysicsObject* object) {
    auto it = std::find_if(m_objects.begin(), m_objects.end(),
        [object](const std::unique_ptr<PhysicsObject>& ptr) {
            return ptr.get() == object;
        });
    
    if (it != m_objects.end()) {
        m_objects.erase(it);
    }
}

void PhysicsWorld::Clear() {
    m_objects.clear();
    DestroyBoundaries();
}

void PhysicsWorld::SetWorldBounds(const Vector2D& worldMin, const Vector2D& worldMax) {
    m_worldMin = worldMin;
    m_worldMax = worldMax;
    
    if (m_hasBoundaries) {
        DestroyBoundaries();
        CreateBoundaries();
    }
}

void PhysicsWorld::EnableBoundaries(bool enable) {
    if (enable && !m_hasBoundaries) {
        CreateBoundaries();
        m_hasBoundaries = true;
    } else if (!enable && m_hasBoundaries) {
        DestroyBoundaries();
        m_hasBoundaries = false;
    }
}

void PhysicsWorld::Step(float deltaTime) {
    // Update all objects
    UpdateObjects(deltaTime);
    
    // Detect and resolve collisions
    DetectAndResolveCollisions();
    
    // Check boundary collisions
    if (m_hasBoundaries) {
        CheckBoundaryCollisions();
    }
}

void PhysicsWorld::UpdateObjects(float deltaTime) {
    for (auto& object : m_objects) {
        object->Update(deltaTime);
    }
}

void PhysicsWorld::DetectAndResolveCollisions() {
    // Check all pairs of objects for collisions
    for (size_t i = 0; i < m_objects.size(); i++) {
        for (size_t j = i + 1; j < m_objects.size(); j++) {
            PhysicsObject* objectA = m_objects[i].get();
            PhysicsObject* objectB = m_objects[j].get();
            
            // Skip if both objects are static
            if (objectA->GetType() == ObjectType::STATIC && 
                objectB->GetType() == ObjectType::STATIC) {
                continue;
            }
            
            CollisionInfo collision = CollisionDetector::DetectCollision(*objectA, *objectB);
            
            if (collision.hasCollision) {
                // Apply collision response to both objects
                objectA->HandleCollision(*objectB, collision.normal, 
                                       collision.penetrationDepth, collision.contactPoint);
                objectB->HandleCollision(*objectA, collision.normal * -1.0f, 
                                       collision.penetrationDepth, collision.contactPoint);
            }
        }
    }
}

void PhysicsWorld::CheckBoundaryCollisions() {
    for (auto& object : m_objects) {
        if (object->GetType() != ObjectType::DYNAMIC) {
            continue; // Only check dynamic objects
        }
        
        // Check each boundary
        for (PhysicsObject* boundary : m_boundaries) {
            CollisionInfo collision = CollisionDetector::DetectCollision(*object, *boundary);
            
            if (collision.hasCollision) {
                object->HandleCollision(*boundary, collision.normal, 
                                      collision.penetrationDepth, collision.contactPoint);
            }
        }
    }
}

void PhysicsWorld::CreateBoundaries() {
    float thickness = 50.0f; // Thickness of boundary walls
    
    float worldWidth = m_worldMax.x - m_worldMin.x;
    float worldHeight = m_worldMax.y - m_worldMin.y;
    
    // Bottom boundary
    auto bottomShape = std::make_unique<RectangleShape>(
        Vector2D(m_worldMin.x + worldWidth/2, m_worldMax.y + thickness/2),
        worldWidth, thickness
    );
    auto bottomBoundary = std::make_unique<PhysicsObject>(std::move(bottomShape), ObjectType::STATIC);
    m_boundaries.push_back(AddObject(std::move(bottomBoundary)));
    
    // Top boundary  
    auto topShape = std::make_unique<RectangleShape>(
        Vector2D(m_worldMin.x + worldWidth/2, m_worldMin.y - thickness/2),
        worldWidth, thickness
    );
    auto topBoundary = std::make_unique<PhysicsObject>(std::move(topShape), ObjectType::STATIC);
    m_boundaries.push_back(AddObject(std::move(topBoundary)));
    
    // Left boundary
    auto leftShape = std::make_unique<RectangleShape>(
        Vector2D(m_worldMin.x - thickness/2, m_worldMin.y + worldHeight/2),
        thickness, worldHeight
    );
    auto leftBoundary = std::make_unique<PhysicsObject>(std::move(leftShape), ObjectType::STATIC);
    m_boundaries.push_back(AddObject(std::move(leftBoundary)));
    
    // Right boundary
    auto rightShape = std::make_unique<RectangleShape>(
        Vector2D(m_worldMax.x + thickness/2, m_worldMin.y + worldHeight/2),
        thickness, worldHeight
    );
    auto rightBoundary = std::make_unique<PhysicsObject>(std::move(rightShape), ObjectType::STATIC);
    m_boundaries.push_back(AddObject(std::move(rightBoundary)));
}

void PhysicsWorld::DestroyBoundaries() {
    for (PhysicsObject* boundary : m_boundaries) {
        RemoveObject(boundary);
    }
    m_boundaries.clear();
}

std::vector<PhysicsObject*> PhysicsWorld::GetObjectsInRadius(const Vector2D& center, float radius) const {
    std::vector<PhysicsObject*> result;
    
    for (const auto& object : m_objects) {
        Vector2D distance = object->GetPosition() - center;
        if (distance.Length() <= radius) {
            result.push_back(object.get());
        }
    }
    
    return result;
}

PhysicsObject* PhysicsWorld::GetObjectAtPoint(const Vector2D& point) const {
    for (const auto& object : m_objects) {
        if (IsPointInShape(point, *object->GetShape())) {
            return object.get();
        }
    }
    return nullptr;
}

bool PhysicsWorld::IsPointInShape(const Vector2D& point, const CollisionShape& shape) const {
    if (shape.GetType() == ShapeType::CIRCLE) {
        const CircleShape& circle = static_cast<const CircleShape&>(shape);
        Vector2D distance = point - circle.GetPosition();
        return distance.Length() <= circle.GetRadius();
    } else if (shape.GetType() == ShapeType::RECTANGLE) {
        const RectangleShape& rectangle = static_cast<const RectangleShape&>(shape);
        return rectangle.ContainsPoint(point);
    }
    return false;
}