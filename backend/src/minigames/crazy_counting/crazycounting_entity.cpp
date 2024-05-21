//
// Created by itssiem on 5/14/24.
//

#include <cstdlib>
#include "crazycounting_entity.h"

CrazyCounting_Entity::CrazyCounting_Entity() : CrazyCounting_Entity((float)std::rand() / RAND_MAX, (float)std::rand() / RAND_MAX) {
}

CrazyCounting_Entity::CrazyCounting_Entity(float x, float y) {
    position = Vector2D(x, y);
    velocity = Vector2D(0, 0);
    heading = Vector2D(0, 0);

    behaviour = new EntityBehaviour(this);
}

void CrazyCounting_Entity::update(unsigned long delta_time) {
    // todo: change this to use steering behaviours
    if (behaviour != nullptr) {
        behaviour->update(delta_time);
    }
    //position.x = (float)std::rand() / RAND_MAX;
    //position.y = (float)std::rand() / RAND_MAX;
}
