#include "business_bailout/business_bailout_minigame.h"
#include "game_update.h"
#include "debug.h"

#include "gen/game_com.pb.h"

auto main() -> int {
    BusinessBailout_Minigame game;

    std::vector<std::string> players;

    players.push_back("kaas");
    players.push_back("stengel");

    BB_GAME gameState = game.start(players);

    GameUpdate<BB_GAME> gameUpdate;
    gameUpdate.gameState = &gameState;
    gameUpdate.reason = GAME_UPDATE_REASON::GAMETICK;
    gameUpdate.deltaTime = 1000;
    game.update(&gameUpdate);
    D(gameState.Serialize());

    gameUpdate.reason = GAME_UPDATE_REASON::PLAYERINPUT;
    gameUpdate.playerName = "kaas";
    gameUpdate.deltaTime = 1200;
    game.update(&gameUpdate);
    D(gameState.Serialize());

    gameUpdate.reason = GAME_UPDATE_REASON::GAMETICK;
    gameUpdate.deltaTime = 1500;
    game.update(&gameUpdate);

    D(gameState.Serialize());

    return 0;
}
