#ifndef PEOPLEPARTY_BACKEND_RIGHT_ON_TIME_MINIGAME_H
#define PEOPLEPARTY_BACKEND_RIGHT_ON_TIME_MINIGAME_H

#include <map>
#include <random>
#include <math.h>
#include "../minigame.h"

struct RightOnTime_Minigame_Player_Data {
    int round_1_diff;
    int round_2_diff;
    int round_3_diff;
    int total_diff;
};

struct RightOnTime_Minigame_Target {
    int round_1_target;
    int round_1_fadeout;
    int round_2_target;
    int round_2_fadeout;
    int round_3_target;
    int round_3_fadeout;
};

class RightOnTime_Minigame : public MiniGame {
private:
    int result_time = 5 SECONDS;
    ThreadTimer introduction_timer;
    ThreadTimer minigame_timer;
    ThreadTimer result_timer;
    std::map<Client*, RightOnTime_Minigame_Player_Data> players;
    RightOnTime_Minigame_Target* target;
    Client* host;
    int current_round = 1;
    int current_phase = 0; // 0 = play minigame, 1 = round result
    int time = 0; // time in ms
public:
    RightOnTime_Minigame(Game *game);
    ~RightOnTime_Minigame();
    void start_introduction() override;
    void start_minigame() override;
    void start_result() override;
    void pause() override;
    void resume() override;
    void update(int delta_time) override;
    std::vector<Client*> getMinigameResult() override;
    void clients_changed(int client_id, bool joined) override { }
    std::string get_display_name() override { return "Right On Time"; };
    std::string get_camel_case_name() override { return "rightOnTime"; };
    std::string get_description() override { return "Stop the clock when the time is right, should be easy right?"; };
private:
    void process_input(const MiniGamePayloadType* payload, Client* from) override;
    void send_payload_data(int client_id, bool is_host, int round_target, bool round_fadeout);
    void send_round_result_data(int client_id);
    void send_result_data(int client_id);
    void introduction_update(int delta_time);
};


#endif //PEOPLEPARTY_BACKEND_RIGHT_ON_TIME_MINIGAME_H
