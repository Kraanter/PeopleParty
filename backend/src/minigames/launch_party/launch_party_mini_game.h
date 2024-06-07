#ifndef PEOPLEPARTY_BACKEND_LAUNCH_PARTY_MINIGAME_H
#define PEOPLEPARTY_BACKEND_LAUNCH_PARTY_MINIGAME_H

#include <map>
#include "../minigame.h"

class LaunchParty_Minigame : public MiniGame {
private:
    int introduction_time = 7 SECONDS;
    int result_time = 10 SECONDS;
    ThreadTimer introduction_timer;
    ThreadTimer minigame_timer;
    ThreadTimer result_timer;
    std::map<Client*, int> players;
    int phase = 0; // 0 = lights slowly go red, 1 = wait for light to turn green, 2 = light is green and wait for players
    int lights_time = 4 SECONDS;
    int wait_for_green_time;
    int wait_time = 5 SECONDS;
public:
    LaunchParty_Minigame(Game *game);
    ~LaunchParty_Minigame() { };
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
    void send_lights_data(int lights_on);
    void send_player_data(Client* player);
    void send_result_data();
    void introduction_update(int delta_time);
};


#endif //PEOPLEPARTY_BACKEND_LAUNCH_PARTY_MINIGAME_H
