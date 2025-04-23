#ifndef PEOPLEPARTY_BACKEND_MOVING_ENTITY_H
#define PEOPLEPARTY_BACKEND_MOVING_ENTITY_H

#include "obstacle_entity.h"

class Moving_Entity {
private:
    bool joystick_is_moving = false;
    int width = 50;
    int height = 50;
public:
    Vector2D position;
    Vector2D velocity;
    Vector2D heading;
    bool is_dead = false;
    int final_score = 0;
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

        if (is_dead) {
            // when dead it moves slowly to the left (velocity)
            // and stops when out of screen
            if (position.x < -width) {
                velocity = Vector2D(0, 0);
            }
        } else {
            // simulate resistance and reduce velocity
            if (!joystick_is_moving && velocity != Vector2D(0, 0)) {
                velocity.x *= 0.90f;
                velocity.y *= 0.75f;
            }

            if (velocity.x < 0.001f && velocity.x > -0.001f) {
                velocity.x = 0;
            }
            if (velocity.y < 0.001f && velocity.y > -0.001f) {
                velocity.y = 0;
            }
        }
    };
    void add_velocity(Vector2D new_velocity) {
        if (is_dead) {
            return;
        }

        new_velocity *= 0.15f; // reduce the velocity to make it more realistic

        // vertical movement is harder
        new_velocity.y *= 0.60f;

        // movement to left is higher than to the right
        if (new_velocity.x < 0) {
            new_velocity.x *= 1.3f;
        } else {
            new_velocity.x *= 0.7f;
        }


        velocity = new_velocity;
        velocity.Truncate(0.2f);
    };
    void change_joystick_is_moving(bool is_moving) {
        joystick_is_moving = is_moving;
    };
    //todo: change this in its own class when refactoring (remove from obstacle as well)
    bool check_colision(Obstacle_Entity* other) {
        // check if the two obstacles are colliding
        if (position.x < other->position.x + other->width &&
            position.x + width > other->position.x &&
            position.y < other->position.y + other->height &&
            position.y + height > other->position.y) {
            return true;
        }
        return false;
    };
};


#endif //PEOPLEPARTY_BACKEND_MOVING_ENTITY_H