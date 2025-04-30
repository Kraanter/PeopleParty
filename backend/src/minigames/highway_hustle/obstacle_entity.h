#ifndef PEOPLEPARTY_BACKEND_OBSTACLE_ENTITY_H
#define PEOPLEPARTY_BACKEND_OBSTACLE_ENTITY_H

#include <utility>
#include "../crazy_counting/steering_behaviour/vector2d.h"
#include <string>
#include <algorithm>

class Obstacle_Entity {
public:
    int width = 50;
    int height = 50;
    std::string id;
    Vector2D position;
    Vector2D velocity;
    int car_type = 0; // (0-?) determines the obstacle vehicle type
public:
    Obstacle_Entity() { Obstacle_Entity(Vector2D(0, 0), Vector2D(0, 0)); };
    Obstacle_Entity(Vector2D pos, Vector2D vel) {
        position = pos;
        velocity = vel;
        car_type = std::rand() % 15; // give random type (0-14)
        set_dimensions(car_type);
    }
    void update(unsigned long delta_time) {
        position += velocity * (delta_time * 2.0f);
    };
    //todo: change this in its own class when refactoring (and remove it from moving as well)
    bool check_colision(Obstacle_Entity* other, int margin = 0) {
        // check if the two obstacles are colliding, with a margin
        if (position.x < other->position.x + other->width + margin &&
            position.x + width > other->position.x - margin &&
            position.y < other->position.y + other->height + margin &&
            position.y + height > other->position.y - margin) {
            return true;
        }
        return false;
    };
private:
    void set_dimensions(int car_type) {
        // sprites have different dimensions
        switch (car_type) {
            default: case 0: case 5: case 6:
                width = 58;
                height = 31;
                break;
            case 1:
                width = 49;
                height = 27;
                break;
            case 2:
                width = 72;
                height = 35;
                break;
            case 3:
                width = 71;
                height = 35;
                break;
            case 4:
                width = 68;
                height = 35;
                break;
            case 7: case 8:
                width = 52;
                height = 30;
                break;
            case 9:
                width = 47;
                height = 27;
                break;
            case 10:
                width = 49;
                height = 28;
                break;
            case 11:
                width = 50;
                height = 28;
                break;
            case 12: case 13:
                width = 49;
                height = 29;
                break;
            case 14:
                width = 43;
                height = 26;
                break;
        }
    }
};


#endif //PEOPLEPARTY_BACKEND_OBSTACLE_ENTITY_H