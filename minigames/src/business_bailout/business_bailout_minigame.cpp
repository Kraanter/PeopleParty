#include <string>
#include <vector>
#include "business_bailout_minigame.h"

BB_GAME BusinessBailout_Minigame::internalStart(std::vector<std::string>& players) {
    BB_GAME gameState;

    // Set a target time for the rocket to keep rising until
    gameState.host.minigame_time = min_duration + (std::rand() % (max_duration - min_duration + 1));

    for (auto &player : players) {
        gameState.players[player].money = -1; 
    }

    gameState.globalData.addMoneyMoment(0, 0);

    return gameState;
};

void BusinessBailout_Minigame::update(Proto_GameUpdate* gameUpdate) {
    std::cerr << "UPDATE" << std::endl;
    BB_GAME gameState;

    gameState.Deserialize(gameUpdate->gamestate());

    getMoneyMoments(&gameState, gameUpdate->deltatime());
    switch(gameUpdate->reason()) {
        case PROTO_GAME_UPDATE_REASON::GAME_TICK:
            break;
        case PROTO_GAME_UPDATE_REASON::PLAYER_INPUT:
            if (gameUpdate->has_playername()) {
                handlePlayerInput(gameUpdate->playername(), &gameState);
            }
            break;
        default:
            break;
    }

    gameState.Serialize(gameUpdate->mutable_gamestate());
}

void BusinessBailout_Minigame::getMoneyMoments(BB_GAME* gameState, int deltaTime) {
    int points = deltaTime / dt;
    gameState->globalData.moneyMoments.clear();
    for (int i = 0; i < points; ++i) {
        const double d = i;
        const double e = 1.1;
        int y = pow(e, d / 10.0f) * 1000 - 1000;
        gameState->globalData.addMoneyMoment(abs(y), i);
    }
}

void BusinessBailout_Minigame::handlePlayerInput(std::string playerName, BB_GAME* gameState) {
    auto playerMap = &gameState->players;
    auto player = playerMap->find(playerName);
    if (player != playerMap->end()) {
        double money = gameState->globalData.moneyMoments.back().money;
        // Set the amount of money to the last increment of money therer is
        player->second.money = money;
    } else {
        D("Could not find \"" << playerName << "\" in list of players (" << playerMap->size() << ")");
    }
}
