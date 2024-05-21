//
// Created by itssiem on 5/14/24.
//

#ifndef PEOPLEPARTY_BACKEND_MINIGAME_H
#define PEOPLEPARTY_BACKEND_MINIGAME_H


#include "../game_state.h"

class Game;

class MiniGame : public GameState {
public:
    MiniGame(Game *game);
    virtual void start() = 0;
protected:
    void finished() override;
    void process_partyprep_input(const PartyPrepPayloadType *payload, Client *from) override {};
};

#endif //PEOPLEPARTY_BACKEND_MINIGAME_H
