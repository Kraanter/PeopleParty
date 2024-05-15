//
// Created by itssiem on 5/14/24.
//

#ifndef PEOPLEPARTY_BACKEND_CRAZYCOUNTING_MINI_GAME_H
#define PEOPLEPARTY_BACKEND_CRAZYCOUNTING_MINI_GAME_H

#include "crazycounting_entity.h"
#include "../minigame.h"
#include <vector>
#include <map>

class CrazyCounting_MiniGame : public Minigame {
public:
    CrazyCounting_MiniGame(int entity_count, const Party& party);
    ~CrazyCounting_MiniGame() = default;
private:
    std::vector<CrazyCounting_Entity> entities;
    std::map<int, int> counting_register;
private:
    void send_entities();
    void send_count(int client_id);
    void update(unsigned long delta_time);
    void process_input(const std::string& payload) override;
};

#endif //PEOPLEPARTY_BACKEND_CRAZYCOUNTING_MINI_GAME_H
