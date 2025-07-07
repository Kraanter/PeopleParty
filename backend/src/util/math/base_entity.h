#ifndef PEOPLEPARTY_BACKEND_BASE_ENTITY_H
#define PEOPLEPARTY_BACKEND_BASE_ENTITY_H

#include "vector2d.h"

class Base_Entity {
public:
    int width = 50;
    int height = 50;
    Vector2D position;
    Vector2D velocity;
public:
    Base_Entity() { Base_Entity(Vector2D(0, 0), Vector2D(0, 0)); };
    Base_Entity(Vector2D pos, Vector2D vel) {
        position = pos;
        velocity = vel;
    }
    bool check_colision(Base_Entity* other, int margin = 0) {
        // check if the two obstacles are colliding, with a margin
        if (position.x < other->position.x + other->width + margin &&
            position.x + width > other->position.x - margin &&
            position.y < other->position.y + other->height + margin &&
            position.y + height > other->position.y - margin) {
            return true;
        }
        return false;
    };
};


#endif //PEOPLEPARTY_BACKEND_BASE_ENTITY_H