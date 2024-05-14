//
// Created by itssiem on 5/14/24.
//

#include "entity.h"

Entity::Entity() {
    position.first = (float)std::rand() / std::RAND_MAX;
    position.second = (float)std::rand() / std::RAND_MAX;
}

Entity::Entity(float x, float y) {
    position.first = x;
    position.second = y;
}

void Entity::update(unsigned long delta_time) {
    // todo: steering behaviours
    position.first = (float)std::rand() / std::RAND_MAX;
    position.second = (float)std::rand() / std::RAND_MAX;
}
