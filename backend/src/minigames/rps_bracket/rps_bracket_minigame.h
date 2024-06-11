//
// Created by itssiem on 6/7/24.
//

#ifndef PEOPLEPARTY_BACKEND_RPS_BRACKET_MINIGAME_H
#define PEOPLEPARTY_BACKEND_RPS_BRACKET_MINIGAME_H

#include "../minigame.h"

enum RPS_Choice {
    ROCK = 0,
    PAPER = 1,
    SCISSORS = 2,
    NONE = 3
};

struct RPS_Match {
    Client* player1;
    Client* player2;
    RPS_Choice player1_choice;
    RPS_Choice player2_choice;
    Client* winner;
    int remaining_time;
};

class RPSBracket_MiniGame : public MiniGame {
public:
    RPSBracket_MiniGame(Game *game);
    void start_introduction() override;
    void start_minigame() override;
    void start_result() override;
    std::vector<Client*> getMinigameResult() override;
    std::string get_display_name() override { return "Rock Paper Scissors Bracket"; }
    std::string get_camel_case_name() override { return "rps_bracket"; }
    std::string get_description() override { return "Rock Paper Scissors Bracket Tournament"; }
    void clients_changed(int client_id, bool joined) override { };
private:
    void process_input(const MiniGamePayloadType* payload, Client* from) override;
    void send_host_update();
    void send_players_update();
    void create_matches(std::vector<Client*> players);
    void update_matches(int delta_time);
    void evaluate_match(RPS_Match* match);
    void promote_winners();
    void update(int delta_time) override;
private:
    std::vector<Client*> players;
    const int match_time = 10 SECONDS;
    std::vector<RPS_Match> matches;
    const int update_interval = 500 MILLISECONDS;
};

#endif //PEOPLEPARTY_BACKEND_RPS_BRACKET_MINIGAME_H
