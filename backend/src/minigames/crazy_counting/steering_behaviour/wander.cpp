#include "wander.h"

#include <cmath>

Wander::Wander() {
    wander_radius = 0.1f;
    wander_distance = 0.2f;
    wander_jitter = 0.08f;
    wander_target = Vector2D(0, 0);
}

Vector2D Wander::Calculate() {
    wander_target += Vector2D(randomClamped() * wander_jitter, randomClamped() * wander_jitter);
    wander_target.Normalize();

    wander_target *= wander_radius;

    return wander_target;
}

float Wander::randomClamped() {
    return ((float)rand() / RAND_MAX) - ((float)rand() / RAND_MAX);
}