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

void MiniGame::process_input(const Message *payload, Client *from) {
    switch(payload->type()) {
        case MessageType::MessageType_MiniGame: {
            auto miniGamePayload = payload->payload_as_MiniGamePayloadType();
            process_input(miniGamePayload, from);
        }
        case MessageType::MessageType_Pause: {
            auto pausePayload = payload->payload_as_PausePayloadType();
            if (pausePayload == nullptr) return;

            if (pausePayload->pause()) {
                pause();
            } else {
                resume();
            }

            //send to everyone
            flatbuffers::FlatBufferBuilder builder;
            auto newPausePayload = CreatePausePayloadType(
                builder,
                pausePayload->pause()
            );

            auto message = CreateMessage(builder, MessageType_Pause, Payload_PausePayloadType, newPausePayload.Union());
            builder.Finish(message);
            int size = builder.GetSize();

            uint8_t* buffer = builder.GetBufferPointer();
            std::string payload_as_string(reinterpret_cast<const char*>(builder.GetBufferPointer()), size);

            game->party->send_message([](Client* client) { return client == client; }, payload_as_string);
        }
    }
}

void MiniGame::send_minigame_introduction(const std::string &minigame_name_camel_case, int time_left, const std::string &minigame_name, const std::string &minigame_description) {
    flatbuffers::FlatBufferBuilder builder;

    auto name = builder.CreateString(minigame_name);
    auto description = builder.CreateString(minigame_description);

    auto payload = CreateMiniGameIntroductionPayload(builder, name, description, time_left);

    auto name_camel_case = builder.CreateString(minigame_name_camel_case);
    auto gameStatePayload = CreateMiniGamePayloadType(
        builder, 
        name_camel_case, 
        GameStateType_MiniGameIntroduction, 
        GameStatePayload_MiniGameIntroductionPayload, 
        payload.Union());

    game->party->send_gamestate([](Client* client) { return client == client; }, builder, gameStatePayload.Union());
}