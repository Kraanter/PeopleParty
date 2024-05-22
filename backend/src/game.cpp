//
// Created by itssiem on 5/15/24.
//

#include "game.h"

Game::Game(Party* party) {
    this->party = party;

    // For now adds all minigames
    miniGames.push(new CrazyCounting_MiniGame(10, this));

        current_gamestate = new PartyPrep(this);
}

void Game::process_input(const MiniGamePayloadType* payload, Client* from) {
    if (current_gamestate != nullptr) {
        current_gamestate->process_input(payload, from);
    }
}

int Game::getPartyId() {
    return party->party_id;
}

const std::vector<Client *> Game::get_clients() {
    return party->get_clients();
}

void Game::clients_changed() {
    if (current_gamestate != nullptr) {
        current_gamestate->clients_changed();
    }
}
