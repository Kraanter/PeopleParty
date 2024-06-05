//
// Created by itssiem on 5/31/24.
//

#ifndef PEOPLEPARTY_BACKEND_BUSINESS_BAILOUT_MINIGAME_H
#define PEOPLEPARTY_BACKEND_BUSINESS_BAILOUT_MINIGAME_H

#include <map>
#include "../minigame.h"

class BusinessBailout_Minigame : public MiniGame {
private:
    int introduction_time = 7 SECONDS;
    int minigame_time = 0;
    int result_time = 5 SECONDS;
    int minigame_start_time;
    ThreadTimer introduction_timer;
    ThreadTimer minigame_timer;
    ThreadTimer result_timer;
    std::vector<Client*> players;
    Client* host = nullptr;
    std::map<Client*, int> player_bail_times;
    unsigned int time = 0;
    int value = 0;
    const int mean = 45 SECONDS;
    const int stddev = 30 SECONDS;
    const int min_duration = 5 SECONDS;
public:
    BusinessBailout_Minigame(Game *game);
    ~BusinessBailout_Minigame();
    void start_introduction() override;
    void start_minigame() override;
    void start_result() override;
    void update(int delta_time) override;
    std::vector<Client*> getMinigameResult() override;
    void clients_changed(int client_id, bool joined) override { }
    std::string get_display_name() override;
    std::string get_camel_case_name() override;
    std::string get_description() override;
private:
    void process_input(const MiniGamePayloadType* payload, Client* from) override;
    void send_host_data();
    void send_player_data(Client* player);
    void introduction_update(int delta_time);
};


#endif //PEOPLEPARTY_BACKEND_BUSINESS_BAILOUT_MINIGAME_H
