#include "leaderboard.h"
#include "../game.h"

Leaderboard::Leaderboard(Game *game) : GameState(game) {
    update_interval = 1 SECONDS;
    remaining_time = 15 SECONDS;
}

void Leaderboard::start() {
    send_leaderboard_information();

    timer.startUpdateTimer(this);
}

void Leaderboard::finished() {
    game->nextGameState<decltype(*this)>();
}

void Leaderboard::process_leaderboard_input(const LeaderboardPayloadType *payload, Client *from)
{
    switch(payload->leaderboardtype()) {
        case LeaderboardType_LeaderboardHostSkip: {
            auto input = payload->leaderboardpayload_as_LeaderboardHostSkipPayload();
            if (input->skip()) {
                finished();
            }
        }
    }
}

void Leaderboard::send_leaderboard_information() {
    flatbuffers::FlatBufferBuilder builder;
    std::vector<flatbuffers::Offset<FBLeaderboardPlayer>> players_buffer;

    for (Client* client: game->get_clients()) {
        if (client->isHost) continue;
        auto name = builder.CreateString(client->name.c_str());
        players_buffer.push_back(CreateFBLeaderboardPlayer(builder, name, game->leaderboard[client]));
    }
    auto entities_vector = builder.CreateVector(players_buffer);

    // Encode payload to binary
    auto payload = CreateLeaderboardInformationPayload(builder, remaining_time, entities_vector);

    auto leaderboardPayload = CreateLeaderboardPayloadType(builder, LeaderboardType_LeaderboardInformation,
                                                      LeaderboardPayload_LeaderboardInformationPayload, payload.Union());

    // Send payload to host
    game->party->send_leaderboard([](Client* client) { return client->party->host == client; }, builder, leaderboardPayload.Union());
}

void Leaderboard::update(int delta_time) {
    remaining_time -= delta_time;
    
    send_leaderboard_information();

    if(remaining_time <= 0) {
        timer.stop();
        Leaderboard::finished();
        return;
    }
}