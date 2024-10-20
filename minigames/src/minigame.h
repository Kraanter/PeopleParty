#ifndef PEOPLEPARTY_BACKEND_MINIGAME_H
#define PEOPLEPARTY_BACKEND_MINIGAME_H

#include "game.h"
#include <vector>

#include <google/protobuf/message.h>
#include "gen/game_com.pb.h"

template <typename H, typename P, typename G>
class MiniGame {
public:
    virtual void update(Proto_GameUpdate* gameUpdate) = 0;
    virtual Game<H, P, G> internalStart(std::vector<std::string>& players) = 0;
};

#endif //PEOPLEPARTY_BACKEND_MINIGAME_H
