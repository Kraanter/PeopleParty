#ifndef ENTITY_BEHAVIOUR_H
#define ENTITY_BEHAVIOUR_H

#include "wander.h"
#include "wall_avoidance.h"
#include "../crazycounting_entity.h"

class Wander;
class WallAvoidance;
class CrazyCounting_Entity;

class EntityBehaviour {
public:
    EntityBehaviour();
    EntityBehaviour(CrazyCounting_Entity* entity);
    void update(float time_delta);
private:
    CrazyCounting_Entity* entity;
    Wander wander_behaviour;
    WallAvoidance* wall_avoidance;
};

#endif //ENTITY_BEHAVIOUR_H