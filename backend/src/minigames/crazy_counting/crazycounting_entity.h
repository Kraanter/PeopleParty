//
// Created by itssiem on 5/14/24.
//

#ifndef PEOPLEPARTY_BACKEND_CRAZYCOUNTING_ENTITY_H
#define PEOPLEPARTY_BACKEND_CRAZYCOUNTING_ENTITY_H

#include <utility>
#include "steering_behaviour/entity_behaviour.h"

class EntityBehaviour;

class CrazyCounting_Entity {
public:
    CrazyCounting_Entity();
    CrazyCounting_Entity(float x, float y);
    void update(unsigned long delta_time);
public:
    Vector2D position;
    Vector2D velocity;
    Vector2D heading;
private:
    EntityBehaviour *behaviour;
};


#endif //PEOPLEPARTY_BACKEND_CRAZYCOUNTING_ENTITY_H
