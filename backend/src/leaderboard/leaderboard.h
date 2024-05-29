#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include "../game_state.h"
#include "../thread_timer.h"

class Game;

class Leaderboard: public GameState {
public:
    Leaderboard(Game *game);
    void clients_changed(int client_id, bool joined) override {};
    void start();
protected:
    void finished() override;
private:
    void send_leaderboard_information();
    void process_leaderboard_input(const LeaderboardPayloadType* payload, Client* from) override;
    void process_partyprep_input(const PartyPrepPayloadType* payload, Client* from) override { };
    void process_input(const MiniGamePayloadType* payload, Client* from) override { };
    void update(int delta_time) override;
private:
    ThreadTimer timer;
    int remaining_time;
};

#endif  // LEADERBOARD_H