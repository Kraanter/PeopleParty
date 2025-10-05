#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include "../game_state.h"
#include "../util/thread_timer.h"
#include <map>
#include <vector>

class Game;

class Leaderboard: public GameState {
public:
    Leaderboard(Game *game);
    void clients_changed(int client_id, bool joined) override { };
    void start();
    void pause() { timer.pause(); };
    void resume() { timer.resume(); };
protected:
    void finished() override;
private:
    void send_leaderboard_information();
    void process_input(const Message* payload, Client* from) override;
    void update(int delta_time) override;
private:
    ThreadTimer timer;
    int remaining_time;
    std::vector<std::string> vote_skipped_players;
};


#endif  // LEADERBOARD_H