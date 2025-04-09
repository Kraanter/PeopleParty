#ifndef PEOPLEPARTY_BACKEND_MOVING_ENTITY_H
#define PEOPLEPARTY_BACKEND_MOVING_ENTITY_H

#include <utility>
#include "../crazy_counting/steering_behaviour/vector2d.h"

class Moving_Entity {
public:
    Vector2D position;
    Vector2D velocity;
    Vector2D heading;
public:
    Moving_Entity() { Moving_Entity(0, 0); };
    Moving_Entity(float x, float y) {
        position = Vector2D(x, y);
        velocity = Vector2D(0, 0);
        heading = Vector2D(0, 0);
    }
    void update(unsigned long delta_time) {
        position += velocity * (delta_time / 1000.0f);
        heading = velocity;
        heading.Normalize();
    };
};


#endif //PEOPLEPARTY_BACKEND_MOVING_ENTITY_H