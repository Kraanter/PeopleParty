//
// Created by itssiem on 5/15/24.
//

#ifndef PEOPLEPARTY_BACKEND_GAME_H
#define PEOPLEPARTY_BACKEND_GAME_H


#include "flatbuffer/messageClass_generated.h"
#include "minigames/minigame.h"
#include "partyprep/partyprep.h"
#include "party.h"
#include "minigames/crazy_counting/crazycounting_mini_game.h"
#include <queue>
#include <map>

class PartyPrep;
class MiniGame;
class Party;
class GameState;
class Client;

class Game {
public:
    Game(Party* party);
    template <typename T>
    void nextGameState() {
        std::cout << "Next game state" << std::endl;
        if (typeid(T) == typeid(MiniGame)) {
            // We just finished a minigame so we change to the leaderboard
            // todo: Create leaderboard
            // return;
        }
        // todo: remove from minigame and add from leaderboard
        if (typeid(T) == typeid(PartyPrep) || typeid(T) == typeid(MiniGame)) {
            if (miniGames.empty()) {
                // todo: Go to the podium
                std::cout << "No more minigames" << std::endl;
                return;
            } else {
                current_gamestate = (GameState *)miniGames.front();
                miniGames.pop();
                ((MiniGame*)current_gamestate)->start();

                // fixme: Infinite loop for now
                miniGames.push(new CrazyCounting_MiniGame(10, this));
            }
        }
    }
    int getPartyId();
    void process_input(const MiniGamePayloadType* payload, Client* from);
    const std::vector<Client*> get_clients();
public:
    std::map<const Client*, int> scores;
    Party *party;
private:
    GameState* current_gamestate;
    std::queue<MiniGame*> miniGames;
};

#endif //PEOPLEPARTY_BACKEND_GAME_H
