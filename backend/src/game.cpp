//
// Created by itssiem on 5/15/24.
//

#include "game.h"
#include "minigames/crazy_counting/crazycounting_mini_game.h"

Game::Game() : Game(nullptr) {}

Game::Game(const Party* party) {
    current_minigame = nullptr;
    this->party = party;

    // For now adds all minigames
    //miniGames.push(new CrazyCounting_MiniGame(10, this));
}

void Game::nextGame() {
    if (!miniGames.empty()) {
        current_minigame = miniGames.front();
        miniGames.pop();

        // Infinite loop for now
        //miniGames.push(new CrazyCounting_MiniGame(10, this));
    }
}

void Game::process_input(const MiniGamePayloadType* payload, Client* from) {
    if (current_minigame != nullptr) {
        current_minigame->process_input(payload, from);
    }
}