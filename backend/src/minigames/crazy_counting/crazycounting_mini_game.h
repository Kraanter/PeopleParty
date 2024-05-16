//
// Created by itssiem on 5/14/24.
//

#ifndef PEOPLEPARTY_BACKEND_CRAZYCOUNTING_MINI_GAME_H
#define PEOPLEPARTY_BACKEND_CRAZYCOUNTING_MINI_GAME_H


#include "../../flatbuffer/messageClass_generated.h"
#include "crazycounting_entity.h"
#include "../minigame.h"
#include "../../timer.h"
#include "../../defines.h"
#include "crazycounting_player.h"
#include <vector>
#include <map>

class CrazyCounting_MiniGame : public MiniGame {
public:
    CrazyCounting_MiniGame(int entity_count, Game* game);
    ~CrazyCounting_MiniGame() = default;
private:
    std::vector<CrazyCounting_Entity> entities;
    std::map<int, CrazyCounting_Player> players;
    Timer timer;
    const int delta_time = 16 MILLISECONDS;
    int remaining_time = 30 SECONDS;
    int time_since_last_time_update = 0;
private:
    void send_entities();
    void send_players_update();
    void send_player_update(int client_id);
    void update(unsigned long delta_time);
    void process_input(const MiniGamePayloadType* payload, Client* from) override;
};

#endif //PEOPLEPARTY_BACKEND_CRAZYCOUNTING_MINI_GAME_H
