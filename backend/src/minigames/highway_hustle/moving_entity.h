#ifndef PEOPLEPARTY_BACKEND_MOVING_ENTITY_H
#define PEOPLEPARTY_BACKEND_MOVING_ENTITY_H

#include <utility>
#include "../crazy_counting/steering_behaviour/vector2d.h"

class Moving_Entity {
private:
    int last_input;
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
        position += velocity * (delta_time * 2.0f);
        heading = velocity;
        heading.Normalize();

        // simulate resistance and reduce velocity
        // only when last_input is higher than 100, to account for the time it takes to send the input
        if (last_input > 100) {
            velocity.x *= 0.90f;
            velocity.y *= 0.75f;
            // TODO: fix stutter bug
            //std::cout << "velocity: " << velocity.x << ", " << velocity.y << std::endl;
            // std::cout << last_input << std::endl;
        }

        if (velocity.x < 0.001f && velocity.x > -0.001f) {
            velocity.x = 0;
        }
        if (velocity.y < 0.001f && velocity.y > -0.001f) {
            velocity.y = 0;
        }

        last_input += delta_time;
    };
    void add_velocity(Vector2D new_velocity) {
        new_velocity *= 0.1f; // reduce the velocity to make it more realistic

        velocity = new_velocity;
        velocity.Truncate(0.1f);

        last_input = 0;
    };
};


#endif //PEOPLEPARTY_BACKEND_MOVING_ENTITY_H