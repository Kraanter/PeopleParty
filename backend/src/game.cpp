//
// Created by itssiem on 5/15/24.
//

#include "game.h"

Game::Game(Party* party) {
    this->party = party;

    // todo: initialize game on party preparation
    current_gamestate = new CrazyCounting_MiniGame(10, this);
    ((MiniGame*)current_gamestate)->start();

    // For now adds all minigames
    miniGames.push(new CrazyCounting_MiniGame(10, this));
}

void Game::process_input(const MiniGamePayloadType* payload, Client* from) {
    if (current_gamestate != nullptr) {
        current_gamestate->process_input(payload, from);
    }
}

const std::vector<Client *> Game::get_clients() {
    return party->get_clients();
}