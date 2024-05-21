#include "entity_behaviour.h"

EntityBehaviour::EntityBehaviour(CrazyCounting_Entity* entity) {
    this->entity = entity;
    wander_behaviour = new Wander();
}

void EntityBehaviour::update(float time_delta) {
    Vector2D SteeringForce = wander_behaviour->Calculate();

    Vector2D acceleration = SteeringForce.clone();
    entity->velocity = entity->velocity + acceleration * time_delta;

    entity->velocity = entity->velocity.truncate(1.0f);

    entity->position = entity->position + entity->velocity * time_delta;

    if (entity->position.lengthSquared() > 0.00000001)
    {
        entity->heading = entity->velocity.normalize();
    } else {
        entity->velocity = Vector2D();
    }

    // for now
    if (entity->position.x > 1.0f) {
        entity->velocity.x = 1.0f;
    }
    if (entity->position.x < 0.0f) {
        entity->velocity.x = 0.0f;
    
    }
    if (entity->position.y > 1.0f) {
        entity->velocity.y = 1.0f;
    }
    if (entity->position.y < 0.0f) {
        entity->velocity.y = 0.0f;
    
    }
}