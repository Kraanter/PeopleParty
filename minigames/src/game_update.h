#ifndef INCLUDE_GPT_GAMEUPDATE_H_
#define INCLUDE_GPT_GAMEUPDATE_H_

#include <string>
#include <optional>

enum GAME_UPDATE_REASON {
    PLAYERINPUT,
    GAMETICK,
};

template <typename TGameState>
class GameUpdate {
public:
    // The state of the game at the moment of this update
    TGameState* gameState;
    // The reason for the update
    GAME_UPDATE_REASON reason;
    // The received payload
    std::string payload;
    // The duration of the game in ms
    int deltaTime;
    // The name of the player if GAME_UPDATE_REASON==PLAYERINPUT
    std::optional<std::string> playerName;
};

#endif  // INCLUDE_GPT_GAMEUPDATE_H_
