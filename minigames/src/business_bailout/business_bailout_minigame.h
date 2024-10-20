#ifndef PEOPLEPARTY_BACKEND_BUSINESS_BAILOUT_MINIGAME_H
#define PEOPLEPARTY_BACKEND_BUSINESS_BAILOUT_MINIGAME_H

#include "../minigame.h"
#include "../defines.h"
#include "busniness_bailout_game_state.h"
#include <vector>

#define BB_GAME Game<BB_HostData, BB_PlayerData, BB_GlobalData>

class BusinessBailout_Minigame : public MiniGame<BB_HostData, BB_PlayerData, BB_GlobalData> {
private:
    const int min_duration = 5 SECONDS;
    const int max_duration = 50 SECONDS;
    const int dt = 100 MILLISECONDS;

public:
    BB_GAME internalStart(std::vector<std::string>& players) override;
    void update(Proto_GameUpdate* gameUpdate) override;

private:
    void getMoneyMoments(BB_GAME* gameState, int deltaTime);
    void handlePlayerInput(std::string playerName, BB_GAME* gameState);
};


#endif //PEOPLEPARTY_BACKEND_BUSINESS_BAILOUT_MINIGAME_H
