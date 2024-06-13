#ifndef PEOPLEPARTY_BACKEND_LAUNCH_PARTY_MINIGAME_H
#define PEOPLEPARTY_BACKEND_LAUNCH_PARTY_MINIGAME_H

#include <map>
#include "../minigame.h"

struct LaunchParty_Player_Data {
    int reaction_time;
    int lag_time;
};

class LaunchParty_Minigame : public MiniGame {
private:
    int result_time;
    ThreadTimer introduction_timer;
    ThreadTimer minigame_timer;
    ThreadTimer result_timer;
    std::map<Client*, LaunchParty_Player_Data> players;
    Client* host;
    int phase = 0; // 0 = lights slowly go red, 1 = wait for light to turn green, 2 = light is green and wait for players
    int lights_time;
    int wait_for_green_time;
    int wait_time;
    bool practice_round = true;
    std::chrono::steady_clock::time_point reaction_time;
public:
    LaunchParty_Minigame(Game *game);
    ~LaunchParty_Minigame();
    void start_introduction() override;
    void start_minigame() override;
    void start_result() override;
    void update(int delta_time) override;
    std::vector<Client*> getMinigameResult() override;
    void clients_changed(int client_id, bool joined) override { }
    std::string get_display_name() override { return "Launch Party"; };
    std::string get_camel_case_name() override { return "launchParty"; };
    std::string get_description() override { return "Test your reaction time!"; };
private:
    void process_input(const MiniGamePayloadType* payload, Client* from) override;
    void send_lights_data(int client_id, int lights_on);
    void send_player_data(Client* player);
    void send_result_data(int client_id);
    void introduction_update(int delta_time);
};


#endif //PEOPLEPARTY_BACKEND_LAUNCH_PARTY_MINIGAME_H
