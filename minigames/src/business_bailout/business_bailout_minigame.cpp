#include <string>
#include <vector>
#include "business_bailout_minigame.h"

BB_GAME BusinessBailout_Minigame::start(std::vector<std::string>& players) {
    BB_GAME gameState;

    // Set a target time for the rocket to keep rising until
    gameState.host.minigame_time = min_duration + (std::rand() % (max_duration - min_duration + 1));

    for (auto &player : players) {
        gameState.players[player].money = -1; 
    }

    gameState.globalData.addMoneyMoment(0, 0);

    return gameState;
};

BB_GAME* BusinessBailout_Minigame::update(GameUpdate<BB_GAME>* gameUpdate) {
    getMoneyMoments(gameUpdate->gameState, gameUpdate->deltaTime);
    switch(gameUpdate->reason) {
        case GAME_UPDATE_REASON::GAMETICK:
            break;
        case GAME_UPDATE_REASON::PLAYERINPUT:
            if (gameUpdate->playerName.has_value()) {
                handlePlayerInput(gameUpdate->playerName.value(), gameUpdate->gameState);
            }
            break;
    }

    return gameUpdate->gameState;
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
