//
// Created by itssiem on 6/7/24.
//

#ifndef PEOPLEPARTY_BACKEND_RPS_BRACKET_MINIGAME_H
#define PEOPLEPARTY_BACKEND_RPS_BRACKET_MINIGAME_H

#include "../minigame.h"
#include <stack>

enum RPS_Choice {
    NONE = 0,
    ROCK = 1,
    PAPER = 2,
    SCISSORS = 3,
};

struct RPS_Match {
    Client* player1;
    Client* player2;
    RPS_Choice player1_choice;
    RPS_Choice player2_choice;
    Client* winner;
    int remaining_time;
    bool sended_result = false;
};

class RPSBracket_MiniGame : public MiniGame {
public:
    RPSBracket_MiniGame(Game *game);
    ~RPSBracket_MiniGame() override;
    void start_introduction() override;
    void start_minigame() override;
    void start_result() override;
    void pause() override;
    void resume() override;
    std::vector<Client*> getMinigameResult() override;
    std::string get_display_name() override { return "Rock Paper Scissors Bracket"; }
    std::string get_camel_case_name() override { return "rpsBracket"; }
    std::string get_description() override { return "Beat All Opponents In A Bracket Style Tournament"; }
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
    void introduction_update(int delta_time);
    void send_match_update(const RPS_Match match);
private:
    ThreadTimer introduction_timer;
    int introduction_time = 7 SECONDS;
    ThreadTimer result_timer;
    const int result_time = 5 SECONDS;
    std::vector<Client*> players;
    const int match_time = 12 SECONDS;
    std::vector<RPS_Match> matches;
    const int update_interval = 500 MILLISECONDS;
    std::stack<Client*> minigame_result;
};

#endif //PEOPLEPARTY_BACKEND_RPS_BRACKET_MINIGAME_H
