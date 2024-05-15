//
// Created by itssiem on 5/14/24.
//

#ifndef PEOPLEPARTY_BACKEND_MINIGAME_H
#define PEOPLEPARTY_BACKEND_MINIGAME_H

#include "../game.h"

class Game;

class MiniGame {
public:
    MiniGame(const Game* game);
    virtual void process_input(const MiniGamePayloadType* payload, Client* from) = 0;
protected:
    const Game* game;
};


#endif //PEOPLEPARTY_BACKEND_MINIGAME_H
