//
// Created by itssiem on 5/14/24.
//

#ifndef PEOPLEPARTY_BACKEND_MINIGAME_H
#define PEOPLEPARTY_BACKEND_MINIGAME_H


#include "../game_state.h"

class Game;

class MiniGame : protected GameState {
public:
    MiniGame(Game *game);
protected:
    void finished() override;
};

#endif //PEOPLEPARTY_BACKEND_MINIGAME_H
