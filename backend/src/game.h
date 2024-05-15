//
// Created by itssiem on 5/15/24.
//

#ifndef PEOPLEPARTY_BACKEND_GAME_H
#define PEOPLEPARTY_BACKEND_GAME_H


#include "party.h"

class Game {
public:
    Game(const Party* party);
public:
    std::vector<const Client*> clients;
    std::map<const Client*, int> scores;
};


#endif //PEOPLEPARTY_BACKEND_GAME_H
