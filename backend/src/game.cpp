//
// Created by itssiem on 5/15/24.
//

#include "game.h"

Game::Game(Party* party) {
    this->party = party;

    MiniGame* minigame;
  
    // std::string randomMinigame = party->settings->GetRandomMiniGame();
    // if (randomMinigame == "crazy_counting") {
    //     minigame = new CrazyCounting_MiniGame(this);
    // } else if (randomMinigame == "memory_mixer") {
    //     minigame = new MemoryMixer_MiniGame(this);
    // } else if (randomMinigame == "buiness_bailout") {
    //     minigame = new BusinessBailout_Minigame(this);
    // } else if (randomMinigame == "rps_bracket") {
    //     minigame = new RPSBracket_MiniGame(this);
    // } else if (randomMinigame == "launch_party") {
    //     minigame = new LaunchParty_Minigame(this);
    // } else {
    //     minigame = new CrazyCounting_MiniGame(this);
    // }

    // std::cout << "Starting game with " << minigame->get_camel_case_name() << std::endl;

    // miniGames.push(minigame);
    // last_minigame = minigame->get_camel_case_name();

    current_gamestate = new PartyPrep(this);
}

void Game::add_minigames() {
    std::vector<MiniGame *> temp_minigames;

    if (party->settings->IsMiniGameEnabled("crazy_counting")) { temp_minigames.push_back(new CrazyCounting_MiniGame(this)); }
    if (party->settings->IsMiniGameEnabled("memory_mixer")) { temp_minigames.push_back(new MemoryMixer_MiniGame(this)); }
    if (party->settings->IsMiniGameEnabled("buiness_bailout")) { temp_minigames.push_back(new BusinessBailout_Minigame(this)); }
    if (party->settings->IsMiniGameEnabled("rps_bracket")) { temp_minigames.push_back(new RPSBracket_MiniGame(this)); }
    if (party->settings->IsMiniGameEnabled("launch_party")) { temp_minigames.push_back(new LaunchParty_Minigame(this)); }

    auto rd = std::random_device{};
    auto rng = std::default_random_engine{rd()};
    std::shuffle(std::begin(temp_minigames), std::end(temp_minigames), rng);

    if (last_minigame == temp_minigames.front()->get_camel_case_name())
    {
        std::reverse(temp_minigames.begin(), temp_minigames.end());
    }

    for (auto minigame : temp_minigames)
    {
        miniGames.push(minigame);
    }
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
    } else {
        leaderboard.erase(client);
        previous_leaderboard.erase(client);
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

void Game::handle_new_minigame() {
    if (last_minigame != "" && party->settings->number_of_rounds != 0 && party->settings->current_round >= party->settings->number_of_rounds)
    {
        // todo: Go to the podium
        std::cout << "No more minigames" << std::endl;
        return;
    }
    else
    {
        if (miniGames.size() < 2)
        {
            add_minigames();
        }

        current_gamestate = (GameState *)miniGames.front();
        miniGames.pop();
        ((MiniGame *)current_gamestate)->start();
        last_minigame = ((MiniGame *)current_gamestate)->get_camel_case_name();
        party->settings->current_round++;
    }
}