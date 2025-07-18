//
// Created by itssiem on 5/15/24.
//

#ifndef PEOPLEPARTY_BACKEND_GAME_H
#define PEOPLEPARTY_BACKEND_GAME_H

#include "flatbuffer/messageClass_generated.h"
#include "minigames/minigame.h"
#include "partyprep/partyprep.h"
#include "partyprep/party_settings.h"
#include "leaderboard/leaderboard.h"
#include "party.h"
#include "minigames/crazy_counting/crazycounting_mini_game.h"
#include "minigames/memory_mixer/memorymixer_mini_game.h"
#include "minigames/business_bailout/business_bailout_minigame.h"
#include "minigames/rps_bracket/rps_bracket_minigame.h"
#include "minigames/launch_party/launch_party_mini_game.h"
#include "minigames/right_on_time/right_on_time_mini_game.h"
#include "minigames/highway_hustle/highway_hustle_mini_game.h"
#include "minigames/marble_mania/marble_mania_mini_game.h"
#include <queue>
#include <map>
#include <algorithm>
#include <random>

class PartyPrep;
class Leaderboard;
class MiniGame;
class Party;
class GameState;
class Client;

class Game
{
public:
    Game(Party *party);
    ~Game();
    template <typename T>
    void nextGameState()
    {
        std::cout << "Next game state" << std::endl;
        if (typeid(T) == typeid(MiniGame))
        {
            // We just finished a minigame so we change to the leaderboard
            current_gamestate = new Leaderboard(this);
            ((Leaderboard *)current_gamestate)->start();
            return;
        }
        if (typeid(T) == typeid(PartyPrep) || typeid(T) == typeid(Leaderboard))
        {
            handle_new_minigame();
        }
    }
    int getPartyId();
    void process_input(const Message *payload, Client *from);
    void clients_changed(int client_id, bool joined);
    const std::vector<Client *> get_clients();
    void update_leaderboard(std::vector<std::pair<Client *, int>> minigame_result); // first is client, second is placement
private:
    void add_minigames();
    void handle_new_minigame();
public:
    std::map<const Client*, std::pair<int, int>> leaderboard; // first is score, seccond is placement
    std::map<const Client*, std::pair<int, int>> previous_leaderboard;
    Party *party;

private:
    GameState *current_gamestate;
    std::queue<MiniGame *> miniGames;
    std::string last_minigame;
};

#endif // PEOPLEPARTY_BACKEND_GAME_H