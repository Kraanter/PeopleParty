#ifndef PEOPLEPARTY_BACKEND_OBSTACLE_ENTITY_H
#define PEOPLEPARTY_BACKEND_OBSTACLE_ENTITY_H

#include <utility>
#include "../crazy_counting/steering_behaviour/vector2d.h"

class Obstacle_Entity {
private:
    int width = 30;
    int height = 30;
public:
    Vector2D position;
    Vector2D velocity;
public:
    Obstacle_Entity() { Obstacle_Entity(Vector2D(0, 0), Vector2D(0, 0)); };
    Obstacle_Entity(Vector2D pos, Vector2D vel) {
        position = pos;
        velocity = vel;
    }
    void update(unsigned long delta_time) {
        position += velocity * (delta_time * 2.0f);
    };
};


#endif //PEOPLEPARTY_BACKEND_OBSTACLE_ENTITY_H