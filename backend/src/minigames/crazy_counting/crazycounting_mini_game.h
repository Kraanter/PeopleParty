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
    CrazyCounting_MiniGame(Game* game);
    ~CrazyCounting_MiniGame();
    void start() override;
    void update(int delta_time) override;
    std::vector<Client*> getMinigameResult() override;
    void clients_changed(int client_id, bool joined) override;
private:
    std::vector<CrazyCounting_Entity*> entities;
    std::map<int, CrazyCounting_Player> players;
    Timer timer;
    int remaining_time;
    int time_since_last_time_update;
    int entity_count;
private:
    void send_entities();
    void send_players_update();
    void send_player_update(int client_id);
    void process_input(const MiniGamePayloadType* payload, Client* from) override;
};

#endif //PEOPLEPARTY_BACKEND_CRAZYCOUNTING_MINI_GAME_H
