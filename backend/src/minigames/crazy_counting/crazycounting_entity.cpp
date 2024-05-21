//
// Created by itssiem on 5/14/24.
//

#include <cstdlib>
#include "crazycounting_entity.h"

CrazyCounting_Entity::CrazyCounting_Entity() : CrazyCounting_Entity((float)std::rand() / RAND_MAX, (float)std::rand() / RAND_MAX) {
}

CrazyCounting_Entity::CrazyCounting_Entity(float x, float y) {
    position.x = x;
    position.y = y;

    behaviour = new EntityBehaviour(this);
}

void CrazyCounting_Entity::update(unsigned long delta_time) {
    behaviour->update(delta_time);
}
