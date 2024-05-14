//
// Created by itssiem on 5/14/24.
//

#ifndef PEOPLEPARTY_BACKEND_GAME_H
#define PEOPLEPARTY_BACKEND_GAME_H

#include "entity.h"
#include "../minigame.h"
#include <vector>
#include <map>

class CrazyCountingGame : public Minigame {
public:
    CrazyCountingGame(int entity_count, const Party& party);
private:
    std::vector<Entity> entities;
    std::map<int, int> counting_register;
private:
    void send_entities();
    void send_count(int client_id);
    void update(unsigned long delta_time);

};

#endif //PEOPLEPARTY_BACKEND_GAME_H
