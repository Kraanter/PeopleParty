#ifndef WANDER_H
#define WANDER_H
#include "vector2d.h"

class Vector2D;

class Wander {
public:
    Wander();
    Vector2D Calculate();
private:
    float wander_radius;
    float wander_distance;
    float wander_jitter;
    Vector2D wander_target;
};

#endif //WANDER_H