//
// Created by itssiem on 5/14/24.
//

#ifndef PEOPLEPARTY_BACKEND_CRAZYCOUNTING_ENTITY_H
#define PEOPLEPARTY_BACKEND_CRAZYCOUNTING_ENTITY_H

#include <utility>

class CrazyCounting_Entity {
public:
    CrazyCounting_Entity();
    CrazyCounting_Entity(float x, float y);
    void update(unsigned long delta_time);
public:
    std::pair<float, float> position;
    std::pair<float, float> velocity;
};


#endif //PEOPLEPARTY_BACKEND_CRAZYCOUNTING_ENTITY_H
