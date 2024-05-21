#include "wander.h"

#include <cmath>

Wander::Wander() {
    wander_radius = 0.01f;
    wander_distance = 0.02f;
    wander_jitter = 0.005f;
    wander_target = Vector2D(0, 0);
}

Vector2D Wander::Calculate() {
    wander_target = wander_target + Vector2D((float)std::rand() / RAND_MAX, (float)std::rand() / RAND_MAX) * wander_jitter;
    wander_target = wander_target.normalize() * wander_radius;

    Vector2D target = wander_target.clone() + Vector2D(wander_distance, 0);

    return target;
}