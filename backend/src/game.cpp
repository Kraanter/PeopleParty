//
// Created by itssiem on 5/15/24.
//

#include "game.h"

Game::Game(Party* party) {
    this->party = party;

    // For now adds all minigames
    miniGames.push(new CrazyCounting_MiniGame(this));

    current_gamestate = new PartyPrep(this);
}

Game::~Game() {
    delete current_gamestate;
    while (!miniGames.empty()) {
        delete miniGames.front();
        miniGames.pop();
    }
    current_gamestate = nullptr;
}

void Game::process_input(const Message* payload, Client* from) {
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

void Game::clients_changed(int client_id, bool joined) {
    if (current_gamestate != nullptr) {
        current_gamestate->clients_changed(client_id, joined);
    }
}

int score(int place, int max_score, int players) {
    if (players == 1) return max_score;
    return (max_score / (players - 1)) * (players - place);
}

void Game::update_leaderboard(std::vector<Client *> minigame_result) {
    for (int i = 1; i <= minigame_result.size(); ++i) {
        Client* client = minigame_result[i - 1];
        leaderboard[client] += score(i, 1000, minigame_result.size());
        std::cout << "place: " << i << " score: " << score(i, 1000, minigame_result.size()) << std::endl;
    }
}



