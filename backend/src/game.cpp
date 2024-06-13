//
// Created by itssiem on 5/15/24.
//

#include "game.h"

Game::Game(Party* party) {
    this->party = party;

    MiniGame* minigame;
  
    int rnd = rand() % 4;
    if (rnd == 0) {
        minigame = new CrazyCounting_MiniGame(this);
    } else if (rnd == 1) {
        minigame = new BusinessBailout_Minigame(this);
    } else if (rnd == 2) {
        minigame = new MemoryMixer_MiniGame(this);
    } else if (rnd == 3) {
        minigame = new LaunchParty_Minigame(this);
    }
    minigame = new CrazyCounting_MiniGame(this);
    miniGames.push(minigame);
    last_minigame = minigame->get_camel_case_name();

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

    Client* client = party->get_client(client_id);

    if (client == nullptr) {
        return;
    }

    if (joined) {
        leaderboard[client] = std::pair<int, int>(0, 1000);
        previous_leaderboard[client] = std::pair<int, int>(0, 0);
    }
}

int score(int place, int max_score, int players) {
    if (players == 1) return max_score;
    return (max_score / (players - 1)) * (players - place);
}

bool cmp(std::pair<const Client*, std::pair<int, int>>& a, std::pair<const Client*, std::pair<int, int>>& b) { 
    return a.second.first > b.second.first; 
} 

void sort(std::map<const Client*, std::pair<int, int>>& M) { 
    std::vector<std::pair<const Client*, std::pair<int, int>>> A; 
    for (auto& it : M) { 
        A.push_back(it); 
    } 
    std::sort(A.begin(), A.end(), cmp);

    for (int i = 0; i < A.size(); i++) { 
        M[A[i].first].second = i + 1; 
    }
} 

void Game::update_leaderboard(std::vector<Client *> minigame_result) {
    previous_leaderboard = leaderboard;
    for (int i = 1; i <= minigame_result.size(); ++i) {
        Client* client = minigame_result[i - 1];
        leaderboard[client].first += score(i, 1000, minigame_result.size());
    }
    sort(leaderboard);
}