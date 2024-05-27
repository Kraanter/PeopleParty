//
// Created by itssiem on 5/14/24.
//

#ifndef PEOPLEPARTY_BACKEND_MINIGAME_H
#define PEOPLEPARTY_BACKEND_MINIGAME_H


#include "../game_state.h"
#include "../timer.h"
#include "../defines.h"

class Game;

class MiniGame : public GameState {
public:
    MiniGame(Game *game);
    virtual void start_introduction() = 0;
    virtual void start_minigame() = 0;
    virtual void start_result() = 0;
    virtual std::vector<Client*> getMinigameResult() = 0;
    void start();
protected:
    void finished() override;
    void process_partyprep_input(const PartyPrepPayloadType *payload, Client *from) override {};
    void process_leaderboard_input(const LeaderboardPayloadType *payload, Client *from) override {};
    void send_minigame_introduction(const std::string &minigame_name_camel_case, const std::string &minigame_name, const std::string &minigame_description);
protected:
    Timer timer;
    int introduction_time = 7 SECONDS;
};

#endif //PEOPLEPARTY_BACKEND_MINIGAME_H
