#include "entity_behaviour.h"
#include <iostream>

EntityBehaviour::EntityBehaviour(CrazyCounting_Entity* entity) {
    this->entity = entity;
    wander_behaviour = new Wander();
}

void EntityBehaviour::update(float time_delta) {
    Vector2D SteeringForce = wander_behaviour->Calculate();

    Vector2D acceleration = SteeringForce / 10000;
    
    // something is going wrong on the next line
    entity->velocity += (acceleration * time_delta);

    entity->velocity.Truncate(0.001f);

    entity->position += (entity->velocity * time_delta);

    std::cout << "Velocity: " << entity->velocity.x << ", " << entity->velocity.y << " Position: " << entity->position.x << ", " << entity->position.y << std::endl;

    if (entity->velocity.Length() > 0.00000001)
    {
        Vector2D new_heading = entity->velocity;
        entity->heading = new_heading.Normalize();
    }

    // for now
    if (entity->position.x > 1.0f) {
        entity->position.x = 1.0f;
    }
    if (entity->position.x < 0.0f) {
        entity->position.x = 0.0f;
    
    }
    if (entity->position.y > 1.0f) {
        entity->position.y = 1.0f;
    }
    if (entity->position.y < 0.0f) {
        entity->position.y = 0.0f;
    
    }
}