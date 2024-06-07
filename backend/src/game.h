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
#include <queue>
#include <map>

class PartyPrep;
class Leaderboard;
class MiniGame;
class Party;
class GameState;
class Client;

class Game {
public:
    Game(Party* party);
    ~Game();
    template <typename T>
    void nextGameState() {
        std::cout << "Next game state" << std::endl;
        if (typeid(T) == typeid(MiniGame)) {
            // We just finished a minigame so we change to the leaderboard
            current_gamestate = new Leaderboard(this);
            ((Leaderboard*)current_gamestate)->start();
            return;
        }
        if (typeid(T) == typeid(PartyPrep) || typeid(T) == typeid(Leaderboard)) {
            if (miniGames.empty()) {
                // todo: Go to the podium
                std::cout << "No more minigames" << std::endl;
                return;
            } else {
                current_gamestate = (GameState *)miniGames.front();
                miniGames.pop();
                ((MiniGame*)current_gamestate)->start();

                // fixme: find a better way of selecting a random minigame
                MiniGame* minigame;
                if (rand() % 2 == 0) {
                    minigame = new CrazyCounting_MiniGame(this);
                } else {
                    minigame = new BusinessBailout_Minigame(this);
                }
                miniGames.push(minigame);
                return;
            }
        }
    }
    int getPartyId();
    void process_input(const Message* payload, Client* from);
    void clients_changed(int client_id, bool joined);
    const std::vector<Client*> get_clients();
    void update_leaderboard(std::vector<Client*> minigame_result);
public:
    std::map<const Client*, int> leaderboard;
    Party *party;
private:
    GameState* current_gamestate;
    std::queue<MiniGame*> miniGames;
};

#endif //PEOPLEPARTY_BACKEND_GAME_H