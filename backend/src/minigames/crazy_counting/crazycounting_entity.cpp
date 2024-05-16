//
// Created by itssiem on 5/14/24.
//

#include <cstdlib>
#include "crazycounting_entity.h"

CrazyCounting_Entity::CrazyCounting_Entity() {
    position.first = (float)std::rand() / RAND_MAX;
    position.second = (float)std::rand() / RAND_MAX;
}

CrazyCounting_Entity::CrazyCounting_Entity(float x, float y) {
    position.first = x;
    position.second = y;
}

void CrazyCounting_Entity::update(unsigned long delta_time) {
    // todo: steering behaviours
    position.first = (float)std::rand() / RAND_MAX;
    position.second = (float)std::rand() / RAND_MAX;
}
