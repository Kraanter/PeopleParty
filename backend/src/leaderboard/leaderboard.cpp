#include "leaderboard.h"
#include "../game.h"
Leaderboard::Leaderboard(Game *game) : GameState(game) {
    update_interval = 1 SECONDS;
    if (game->party->settings->game_finished) {
        // podium leaderboard
        remaining_time = 6000 SECONDS;
    } else {
        // normal leaderboard
        remaining_time = 10 SECONDS;
    }
}

void Leaderboard::start() {
    send_leaderboard_information();

    timer.setInterval([this]() { update(update_interval); }, update_interval);
}

void Leaderboard::finished() {
    game->nextGameState<decltype(*this)>();
}

void Leaderboard::process_input(const Message *payload, Client *from) {
    switch(payload->type()) {
        case MessageType::MessageType_Leaderboard: {
            auto leaderboardPayload = payload->payload_as_LeaderboardPayloadType();
            switch(leaderboardPayload->leaderboardtype()) {
                case LeaderboardType_LeaderboardHostSkip: {
                    auto input = leaderboardPayload->leaderboardpayload_as_LeaderboardHostSkipPayload();
                    if (input->skip()) {
                        timer.clear();
                        finished();
                    }
                    break;
                }
            }
            break;
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

void Leaderboard::send_leaderboard_information() {
    flatbuffers::FlatBufferBuilder builder;
    std::vector<flatbuffers::Offset<FBLeaderboardPlayer>> players_buffer;

    for (Client* client: game->get_clients()) {
        if (client->isHost) continue;
        if (client == nullptr) continue;

        auto name = builder.CreateString(client->name.c_str());
        int delta_score = game->leaderboard[client].first - game->previous_leaderboard[client].first;

        int delta_position;
        if (game->previous_leaderboard[client].second == 0 || game->previous_leaderboard[client].second == 1000 
            || game->leaderboard[client].second == 1000) {
            delta_position = 0;
        } else {
            delta_position = game->previous_leaderboard[client].second - game->leaderboard[client].second;
        }

        players_buffer.push_back(CreateFBLeaderboardPlayer(builder, name, game->leaderboard[client].second, game->leaderboard[client].first, delta_score, delta_position));
    }

    auto entities_vector = builder.CreateVector(players_buffer);

    // Encode payload to binary
    auto payload = CreateLeaderboardInformationPayload(builder, remaining_time, game->party->settings->game_finished, entities_vector);

    auto leaderboardPayload = CreateLeaderboardPayloadType(builder, LeaderboardType_LeaderboardInformation,
                                                      LeaderboardPayload_LeaderboardInformationPayload, payload.Union());

    // Send payload to host
    game->party->send_leaderboard([](Client* client) { return true; }, builder, leaderboardPayload.Union());
}


void Leaderboard::update(int delta_time) {
    remaining_time -= delta_time;
    
    send_leaderboard_information();

    if(remaining_time <= 0) {
        timer.clear();
        finished();
        return;
    }
}