#ifndef PEOPLEPARTY_BACKEND_MARBLE_MANIA_MINI_GAME_H
#define PEOPLEPARTY_BACKEND_MARBLE_MANIA_MINI_GAME_H


#include "../../flatbuffer/messageClass_generated.h"
#include "../minigame.h"
#include "marble_mania_map.h"
#include <vector>
#include <math.h>
#include <map>

class MarbleManiaMap;

class MarbleMania_MiniGame : public MiniGame {
private:
    int target_fps = 20;
    int result_time = 5 SECONDS;
    ThreadTimer introduction_timer;
    ThreadTimer minigame_timer;
    ThreadTimer result_timer;
    
    // Game-specific properties
    MarbleManiaMap* map;
    bool placementPhaseComplete;
    float placementTimeLeft;
public:
    MarbleMania_MiniGame(Game* game);
    ~MarbleMania_MiniGame();
    void start_introduction() override;
    void start_minigame() override;
    void start_result() override;
    void pause() override;
    void resume() override;
    std::vector<std::pair<Client *, int>> getMinigameResult() override;
    void clients_changed(int client_id, bool joined) override { };
    std::string get_display_name() override { return "Marble Mania"; };
    std::string get_camel_case_name() override { return "marbleMania"; };
    std::string get_description() override { return "Drop your marble and watch it race to the finish line!"; };
private:
    void update(int delta_time) override;
    void process_input(const MiniGamePayloadType* payload, Client* from) override;
    void send_host_update();
    void send_player_update(Client *client);
    void send_result_data(int client_id);
    void introduction_update(int delta_time);
};

#endif //PEOPLEPARTY_BACKEND_MARBLE_MANIA_MINI_GAME_H
