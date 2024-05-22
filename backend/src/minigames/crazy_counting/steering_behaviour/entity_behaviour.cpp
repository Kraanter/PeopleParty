#include "entity_behaviour.h"
#include <iostream>

EntityBehaviour::EntityBehaviour() {
    entity = nullptr;
}

EntityBehaviour::EntityBehaviour(CrazyCounting_Entity* entity) {
    this->entity = entity;
    wander_behaviour = Wander();
    wall_avoidance = new WallAvoidance(entity);
}

void EntityBehaviour::update(float time_delta) {
    if (entity == nullptr) {
        return;
    }
    Vector2D SteeringForce = wander_behaviour.Calculate();
    //Vector2D forceWallAvoidance = wall_avoidance->Calculate() * 10.0f;

    Vector2D acceleration = SteeringForce / 1000000;
    
    entity->velocity += (acceleration * time_delta);

    entity->velocity.Truncate(0.0005f);

    entity->position += (entity->velocity * time_delta);

    if (entity->velocity.LengthSq() > 0.00000001)
    {
        entity->heading = entity->velocity;
        entity->heading.Normalize();
    }

    // for now
    if (entity->position.x > 1.0f) {
        entity->position.x = 1.0f;
        entity->velocity.x = 0.0f;
        wander_behaviour.FlipX();
    } else if (entity->position.x < 0.0f) {
        entity->position.x = 0.0f;
        entity->velocity.x = 0.0f;
        wander_behaviour.FlipX();
    } else if (entity->position.y > 1.0f) {
        entity->position.y = 1.0f;
        entity->velocity.y = 0.0f;
        wander_behaviour.FlipY();
    } else if (entity->position.y < 0.0f) {
        entity->position.y = 0.0f;
        entity->velocity.y = 0.0f;
        wander_behaviour.FlipY();
    }
}