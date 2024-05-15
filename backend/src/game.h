//
// Created by itssiem on 5/15/24.
//

#ifndef PEOPLEPARTY_BACKEND_GAME_H
#define PEOPLEPARTY_BACKEND_GAME_H


#include "flatbuffer/messageClass_generated.h"
#include "party.h"
#include "minigames/minigame.h"
#include <queue>
#include <map>

class MiniGame;
class Party;

class Game {
public:
    Game();
    Game(const Party* party);
    void nextGame();
    void process_input(const MiniGamePayloadType* payload, Client* from);
public:
    std::vector<Client*> clients;
    std::map<const Client*, int> scores;
private:
    const Party* party;
    MiniGame* current_minigame;
    std::queue<MiniGame*> miniGames;
};


#endif //PEOPLEPARTY_BACKEND_GAME_H
