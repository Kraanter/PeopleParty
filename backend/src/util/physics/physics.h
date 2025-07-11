#ifndef PHYSICS_H
#define PHYSICS_H

// Core physics system
#include "physics_world.h"
#include "rigid_body.h"
#include "collision.h"

// Marble-specific physics
#include "marble.h"

// Utilities
#include "physics_constants.h"
#include "spatial_grid.h"

// Math utilities (from existing codebase)
#include "../math/vector2d.h"
#include "../math/matrix2d.h"
#include "../math/base_entity.h"

/**
 * Main Physics namespace containing all physics-related functionality
 * for the PeopleParty marble minigame.
 * 
 * Usage example:
 * 
 * // Create a physics world
 * PhysicsWorld world(Vector2D(-400, -300), Vector2D(400, 300));
 * world.SetGravity(Vector2D(0, 981)); // 981 units/s^2 downward
 * 
 * // Create marbles
 * Marble* marble1 = Marble::CreateNormalMarble(Vector2D(0, -200));
 * Marble* marble2 = Marble::CreateBouncyMarble(Vector2D(50, -200));
 * 
 * // Add to world
 * world.CreateBody(marble1->GetPosition(), marble1->GetMass(), marble1->GetRadius());
 * world.CreateBody(marble2->GetPosition(), marble2->GetMass(), marble2->GetRadius());
 * 
 * // Add boundaries
 * world.AddBoundary(Vector2D(-400, 300), Vector2D(400, 300)); // Floor
 * world.AddBoundary(Vector2D(-400, -300), Vector2D(-400, 300)); // Left wall
 * world.AddBoundary(Vector2D(400, -300), Vector2D(400, 300)); // Right wall
 * 
 * // Simulation loop
 * float deltaTime = 1.0f / 60.0f; // 60 FPS
 * world.Step(deltaTime);
 */

namespace Physics {
    
    // Factory functions for common physics setups
    inline PhysicsWorld* CreateMarbleWorld(const Vector2D& worldMin, const Vector2D& worldMax) {
        PhysicsWorld* world = new PhysicsWorld(worldMin, worldMax);
        world->SetGravity(Vector2D(0, DEFAULT_GRAVITY * 100)); // Scale for game units
        world->SetDamping(DEFAULT_DAMPING);
        return world;
    }
    
    inline void AddWorldBoundaries(PhysicsWorld* world, const Vector2D& worldMin, const Vector2D& worldMax) {
        if (!world) return;
        
        // Floor
        world->AddBoundary(Vector2D(worldMin.x, worldMax.y), Vector2D(worldMax.x, worldMax.y));
        
        // Left wall
        world->AddBoundary(Vector2D(worldMin.x, worldMin.y), Vector2D(worldMin.x, worldMax.y));
        
        // Right wall
        world->AddBoundary(Vector2D(worldMax.x, worldMin.y), Vector2D(worldMax.x, worldMax.y));
        
        // Ceiling (optional)
        // world->AddBoundary(Vector2D(worldMin.x, worldMin.y), Vector2D(worldMax.x, worldMin.y));
    }
    
    inline void SpawnRandomMarbles(PhysicsWorld* world, int count, const Vector2D& spawnArea) {
        if (!world) return;
        
        for (int i = 0; i < count; ++i) {
            Vector2D position = RandomVector2D(
                spawnArea.x - spawnArea.x * 0.5f, spawnArea.x + spawnArea.x * 0.5f,
                spawnArea.y - spawnArea.y * 0.5f, spawnArea.y + spawnArea.y * 0.5f
            );
            
            // Random marble type
            MarbleType type = static_cast<MarbleType>(rand() % 5);
            
            MarbleBase* marble = nullptr;
            switch (type) {
                case MarbleType::NORMAL:
                    marble = MarbleBase::CreateNormalMarble(position);
                    break;
                case MarbleType::HEAVY:
                    marble = MarbleBase::CreateHeavyMarble(position);
                    break;
                case MarbleType::LIGHT:
                    marble = MarbleBase::CreateLightMarble(position);
                    break;
                case MarbleType::BOUNCY:
                    marble = MarbleBase::CreateBouncyMarble(position);
                    break;
                case MarbleType::STICKY:
                    marble = MarbleBase::CreateStickyMarble(position);
                    break;
            }
            
            if (marble) {
                world->CreateBody(marble->GetPosition(), marble->GetMass(), marble->GetRadius());
            }
        }
    }
}

#endif // PHYSICS_H
