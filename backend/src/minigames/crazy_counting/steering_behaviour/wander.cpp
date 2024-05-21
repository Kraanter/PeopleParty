#include "wander.h"

#include <cmath>

Wander::Wander() {
    wander_radius = 0.005f;
    wander_distance = 0.01f;
    wander_jitter = 0.0005f;
    wander_target = Vector2D(0, 0);
}

Vector2D Wander::Calculate() {
    wander_target += Vector2D(randomClamped() * wander_jitter, randomClamped() * wander_jitter);
    wander_target.Normalize();

    wander_target *= wander_radius;

    Vector2D targetLocal = wander_target + Vector2D(wander_distance, 0);

    return targetLocal;
}

float Wander::randomClamped() {
    return ((float)rand() / RAND_MAX) - ((float)rand() / RAND_MAX);
}