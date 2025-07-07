#ifndef WALL_AVOIDANCE_H
#define WALL_AVOIDANCE_H

#include "wall2d.h"
#include "../../../util/math/vector2d.h"
#include "../../../util/math/matrix2d.h"
#include <vector>

#include "../crazycounting_entity.h"

class CrazyCounting_Entity;

class WallAvoidance {
public:
    WallAvoidance();
    WallAvoidance(CrazyCounting_Entity* entity);
    Vector2D Calculate();
private:
    std::vector<Wall2D> walls;
    std::vector<Vector2D> feelers;
    CrazyCounting_Entity* entity;
private:
    void CreateFeelers();
};

#endif //WALL_AVOIDANCE_H