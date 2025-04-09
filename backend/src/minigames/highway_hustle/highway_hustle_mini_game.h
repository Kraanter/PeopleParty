#ifndef PEOPLEPARTY_BACKEND_HIGHWAY_HUSTLE_MINI_GAME_H
#define PEOPLEPARTY_BACKEND_HIGHWAY_HUSTLE_MINI_GAME_H


#include "../../flatbuffer/messageClass_generated.h"
#include "../minigame.h"
#include "highway_hustle_map.h"
#include <vector>
#include <map>

class HighwayHustle_Map;

class HighwayHustle_MiniGame : public MiniGame {
private:
    int result_time = 5 SECONDS;
    ThreadTimer introduction_timer;
    ThreadTimer minigame_timer;
    ThreadTimer result_timer;
    HighwayHustle_Map* map;
public:
    HighwayHustle_MiniGame(Game* game);
    ~HighwayHustle_MiniGame();
    void start_introduction() override;
    void start_minigame() override;
    void start_result() override;
    void pause() override;
    void resume() override;
    std::vector<std::pair<Client *, int>> getMinigameResult() override;
    void clients_changed(int client_id, bool joined) override { };
    std::string get_display_name() override { return "Highway Hustle"; };
    std::string get_camel_case_name() override { return "highwayHustle"; };
    std::string get_description() override { return "Manouver through the trafic, watch out for the other cars!"; };
    private:
    void update(int delta_time) override;
    void process_input(const MiniGamePayloadType* payload, Client* from) override;
    void send_host_update();
    void send_players_update();
    void introduction_update(int delta_time);
};

#endif //PEOPLEPARTY_BACKEND_HIGHWAY_HUSTLE_MINI_GAME_H
