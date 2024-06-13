//
// Created by itssiem on 5/15/24.
//

#ifndef PEOPLEPARTY_BACKEND_GAME_H
#define PEOPLEPARTY_BACKEND_GAME_H

#include "flatbuffer/messageClass_generated.h"
#include "minigames/minigame.h"
#include "partyprep/partyprep.h"
#include "leaderboard/leaderboard.h"
#include "party.h"
#include "minigames/crazy_counting/crazycounting_mini_game.h"
#include "minigames/memory_mixer/memorymixer_mini_game.h"
#include "minigames/business_bailout/business_bailout_minigame.h"
#include "minigames/rps_bracket/rps_bracket_minigame.h"
#include "minigames/launch_party/launch_party_mini_game.h"
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
            if (miniGames.empty())
            {
                // todo: Go to the podium
                std::cout << "No more minigames" << std::endl;
                return;
            }
            else
            {
                current_gamestate = (GameState *)miniGames.front();
                miniGames.pop();
                ((MiniGame *)current_gamestate)->start();
                last_minigame = ((MiniGame *)current_gamestate)->get_camel_case_name();

                if (miniGames.size() < 2)
                {
                    std::vector<MiniGame *> temp_minigames;
                    temp_minigames.push_back(new CrazyCounting_MiniGame(this));
                    temp_minigames.push_back(new BusinessBailout_Minigame(this));
                    temp_minigames.push_back(new MemoryMixer_MiniGame(this));
                    temp_minigames.push_back(new LaunchParty_Minigame(this));
                    temp_minigames.push_back(new RPSBracket_MiniGame(this));

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
            }
        }
    }
    int getPartyId();
    void process_input(const Message *payload, Client *from);
    void clients_changed(int client_id, bool joined);
    const std::vector<Client *> get_clients();
    void update_leaderboard(std::vector<Client *> minigame_result);

public:
    std::map<const Client *, int> leaderboard;
    Party *party;

private:
    GameState *current_gamestate;
    std::queue<MiniGame *> miniGames;
    std::string last_minigame;
};

#endif // PEOPLEPARTY_BACKEND_GAME_H