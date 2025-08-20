#ifndef PEOPLEPARTY_BACKEND_OBSTACLE_ENTITY_H
#define PEOPLEPARTY_BACKEND_OBSTACLE_ENTITY_H

#include <utility>
#include "../../util/math/base_entity.h"
#include <string>
#include <algorithm>

class Obstacle_Entity : public Base_Entity {
public:
    std::string id;
    Vector2D position;
    Vector2D velocity;
    int car_type = 0; // (0-?) determines the obstacle vehicle type
public:
    Obstacle_Entity() : Base_Entity() {};
    Obstacle_Entity(Vector2D pos, Vector2D vel) : Base_Entity(pos, vel) {
        car_type = std::rand() % 13; // give random type (0-12)
        set_dimensions(car_type);
    };
    void update(unsigned long delta_time) {
        position += velocity * (delta_time * 2.0f);
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
            case 7:
                width = 52;
                height = 30;
                break;
            case 8:
                width = 47;
                height = 27;
                break;
            case 9:
                width = 49;
                height = 28;
                break;
            case 10: case 11:
                width = 49;
                height = 29;
                break;
            case 12:
                width = 43;
                height = 26;
                break;
        }

        width *= 1.5f;
        height *= 1.5f;
    }
};


#endif //PEOPLEPARTY_BACKEND_OBSTACLE_ENTITY_H