//
// Created by itssiem on 5/14/24.
//

#include "minigame.h"
#include "../game.h"

MiniGame::MiniGame(Game *game) : GameState(game) {
}

void MiniGame::finished() {
    game->nextGameState<decltype(*this)>();
}