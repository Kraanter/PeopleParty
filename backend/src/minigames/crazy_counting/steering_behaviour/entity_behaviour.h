#ifndef ENTITY_BEHAVIOUR_H
#define ENTITY_BEHAVIOUR_H

#include "wander.h"
#include "../crazycounting_entity.h"

class Wander;
class CrazyCounting_Entity;

class EntityBehaviour {
public:
    EntityBehaviour(CrazyCounting_Entity* entity);
    void update(float time_delta);
private:
    CrazyCounting_Entity* entity;
    Wander* wander_behaviour;
};

#endif //ENTITY_BEHAVIOUR_H