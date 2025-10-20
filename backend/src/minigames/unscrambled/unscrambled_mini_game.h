#ifndef PEOPLEPARTY_BACKEND_UNSCRAMBLED_MINIGAME_H
#define PEOPLEPARTY_BACKEND_UNSCRAMBLED_MINIGAME_H

#include <map>
#include <random>
#include <math.h>
#include "../minigame.h"

class Unscrambled_Minigame : public MiniGame {
private:
    int max_rounds = 4;
    int result_time = 5 SECONDS;
    ThreadTimer introduction_timer;
    ThreadTimer minigame_timer;
    ThreadTimer result_timer;
    std::map<Client*, int> players; // score: int = amount of correct words
    int current_round = 1;
    int current_phase = 0; // 0 = play minigame, 1 = round result
    int time = 0; // time in ms
public:
    Unscrambled_Minigame(Game *game);
    ~Unscrambled_Minigame();
    void start_introduction() override;
    void start_minigame() override;
    void start_result() override;
    void pause() override;
    void resume() override;
    void update(int delta_time) override;
    std::vector<std::pair<Client *, int>> getMinigameResult() override;
    void clients_changed(int client_id, bool joined) override { }
    std::string get_display_name() override { return "Unscrambled"; };
    std::string get_camel_case_name() override { return "unscrambled"; };
    std::string get_description() override { return "Unscramble the words before the time runs out!"; };
private:
    void process_input(const MiniGamePayloadType* payload, Client* from) override;
    void send_host_payload_data(int client_id);
    void send_player_payload_data(int client_id);
    void send_round_result_data(int client_id);
    void send_result_data(int client_id);
    void introduction_update(int delta_time);
};


#endif //PEOPLEPARTY_BACKEND_UNSCRAMBLED_MINIGAME_H
