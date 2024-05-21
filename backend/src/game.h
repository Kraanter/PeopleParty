//
// Created by itssiem on 5/15/24.
//

#ifndef PEOPLEPARTY_BACKEND_GAME_H
#define PEOPLEPARTY_BACKEND_GAME_H


#include "flatbuffer/messageClass_generated.h"
#include "minigames/minigame.h"
#include "party.h"
#include "minigames/crazy_counting/crazycounting_mini_game.h"
#include <queue>
#include <map>

class MiniGame;
class Party;
class GameState;
class Client;

class Game {
public:
    Game(Party* party);
    template <typename T>
    void nextGameState() {
        if (std::is_base_of<MiniGame, T>::value) {
            // We just finished a minigame so we change to the leaderboard
            // todo: Create leaderboard
            return;
        }
        if (miniGames.empty()) {
            // todo: Go to the podium
            return;
        }

        current_gamestate = (GameState *)miniGames.front();
        miniGames.pop();

        // fixme: Infinite loop for now
        miniGames.push(new CrazyCounting_MiniGame(10, this));
    }
    void process_input(const MiniGamePayloadType* payload, Client* from);
public:
    std::vector<Client*> clients;
    std::map<const Client*, int> scores;
    Party *party;
private:
    GameState* current_gamestate;
    std::queue<MiniGame*> miniGames;
};


#endif //PEOPLEPARTY_BACKEND_GAME_H
