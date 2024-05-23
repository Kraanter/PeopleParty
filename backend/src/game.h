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
    template <typename T>
    void nextGameState() {
        std::cout << "Next game state" << std::endl;
        if (typeid(T) == typeid(MiniGame)) {
            // We just finished a minigame so we change to the leaderboard
            current_gamestate = new Leaderboard(this);
            ((Leaderboard*)current_gamestate)->start();
            return;
        }
        // todo: remove from minigame and add from leaderboard
        if (typeid(T) == typeid(PartyPrep) || typeid(T) == typeid(Leaderboard)) {
            if (miniGames.empty()) {
                // todo: Go to the podium
                std::cout << "No more minigames" << std::endl;
                return;
            } else {
                current_gamestate = (GameState *)miniGames.front();
                miniGames.pop();
                ((MiniGame*)current_gamestate)->start();

                // fixme: Infinite loop for now
                miniGames.push(new CrazyCounting_MiniGame(this));
                return;
            }
        }
    }
    int getPartyId();
    void process_input(const MiniGamePayloadType* payload, Client* from);
    void process_partyprep_input(const PartyPrepPayloadType* payload, Client* from);
    void process_leaderboard_input(const LeaderboardPayloadType* payload, Client* from);
    void clients_changed();
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
