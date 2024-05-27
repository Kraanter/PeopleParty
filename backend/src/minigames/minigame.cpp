//
// Created by itssiem on 5/14/24.
//

#include "minigame.h"
#include "../game.h"

MiniGame::MiniGame(Game *game) : GameState(game) {
}

void MiniGame::finished() {
    game->update_leaderboard(getMinigameResult());
    game->nextGameState<decltype(*this)>();
}

void MiniGame::start() {
    start_introduction();
}

void MiniGame::send_minigame_introduction(const std::string &minigame_name_camel_case, const std::string &minigame_name, const std::string &minigame_description) {
    flatbuffers::FlatBufferBuilder builder;

    auto name = builder.CreateString(minigame_name);
    auto description = builder.CreateString(minigame_description);

    auto payload = CreateMiniGameIntroductionPayload(builder, name, description);

    auto name_camel_case = builder.CreateString(minigame_name_camel_case);
    auto gameStatePayload = CreateMiniGamePayloadType(
        builder, 
        name_camel_case, 
        GameStateType_MiniGameIntroduction, 
        GameStatePayload_MiniGameIntroductionPayload, 
        payload.Union());

    // todo: also send to normal clients
    game->party->send_gamestate([](Client* client) { return client->party->host == client; }, builder, gameStatePayload.Union());
}