//
// Created by itssiem on 5/14/24.
//

#include "minigame.h"

MiniGame::MiniGame(Game* game) {
    this->game = game;
}

void MiniGame::finish() {
    game->nextGame();
}