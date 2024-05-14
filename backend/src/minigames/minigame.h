//
// Created by itssiem on 5/14/24.
//

#ifndef PEOPLEPARTY_BACKEND_MINIGAME_H
#define PEOPLEPARTY_BACKEND_MINIGAME_H

#include "../party.h"

class Minigame {
public:
    Minigame(const Party& party);
    virtual void process_input(const std::string& payload) = 0;
protected:
    Party party;
};


#endif //PEOPLEPARTY_BACKEND_MINIGAME_H
