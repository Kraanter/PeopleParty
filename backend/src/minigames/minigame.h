//
// Created by itssiem on 5/14/24.
//

#ifndef PEOPLEPARTY_BACKEND_MINIGAME_H
#define PEOPLEPARTY_BACKEND_MINIGAME_H


#include "../game_state.h"
#include "../util/thread_timer.h"
#include "../util/defines.h"

class Game;

class MiniGame : public GameState {
public:
    MiniGame(Game *game);
    virtual void start_introduction() = 0;
    virtual void start_minigame() = 0;
    virtual void start_result() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual std::vector<std::pair<Client *, int>> getMinigameResult() = 0; // first is client, second is placement
    void start();
public:
    virtual std::string get_display_name() = 0;
    virtual std::string get_camel_case_name() = 0;
    virtual std::string get_description() = 0;
protected:
    virtual void process_input(const MiniGamePayloadType* payload, Client* from) = 0;
    void finished() override;
    void process_input(const Message *payload, Client *from) override;
    void send_minigame_introduction(const std::string &minigame_name_camel_case, int time_left, const std::string &minigame_name, const std::string &minigame_description);
public:
    int min_players = 2;
    int max_players = -1; // -1 = no limit
protected:
    ThreadTimer timer;
    int introduction_time = 7 SECONDS;
};

#endif //PEOPLEPARTY_BACKEND_MINIGAME_H
