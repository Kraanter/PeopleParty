//
// Created by itssiem on 5/14/24.
//

#ifndef PEOPLEPARTY_BACKEND_CRAZYCOUNTING_MINI_GAME_H
#define PEOPLEPARTY_BACKEND_CRAZYCOUNTING_MINI_GAME_H


#include "../../flatbuffer/messageClass_generated.h"
#include "crazycounting_entity.h"
#include "../minigame.h"
#include "crazycounting_player.h"
#include <vector>
#include <map>

class CrazyCounting_MiniGame : public MiniGame {
public:
    CrazyCounting_MiniGame(Game* game);
    ~CrazyCounting_MiniGame();
    void start_introduction() override;
    void start_minigame() override;
    void start_result() override;
    void pause() override;
    void resume() override;
    void update(int delta_time) override;
    void introduction_update(int delta_time);
    std::vector<Client*> getMinigameResult() override;
    void clients_changed(int client_id, bool joined) override;
    std::string get_display_name() override;
    std::string get_camel_case_name() override;
    std::string get_description() override;
private:
    const int distribution_mean = 20;
    const int distribution_variance = 4;
    std::vector<CrazyCounting_Entity*> entities;
    std::map<int, CrazyCounting_Player> players;
    ThreadTimer introduction_timer;
    ThreadTimer results_timer;
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
