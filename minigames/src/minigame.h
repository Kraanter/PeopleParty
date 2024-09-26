#ifndef PEOPLEPARTY_BACKEND_MINIGAME_H
#define PEOPLEPARTY_BACKEND_MINIGAME_H

#include "game.h"
#include "game_update.h"
#include <vector>

template <typename H, typename P, typename G>
class MiniGame {
public:
    virtual Game<H, P, G>* update(GameUpdate<Game<H, P, G>>* gameUpdate) = 0;
    virtual Game<H, P, G> start(std::vector<std::string>& players) = 0;
};

#endif //PEOPLEPARTY_BACKEND_MINIGAME_H
