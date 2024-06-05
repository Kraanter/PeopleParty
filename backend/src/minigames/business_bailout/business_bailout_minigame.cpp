//
// Created by itssiem on 5/31/24.
//

#include <random>
#include "business_bailout_minigame.h"
#include "../../game.h"

BusinessBailout_Minigame::BusinessBailout_Minigame(Game *game) : MiniGame(game) {
    std::mt19937 rng(std::random_device{}());
    std::binomial_distribution distribution(mean, stddev);
    while (minigame_time < min_duration) {
        minigame_time = distribution(rng);
    }
}

BusinessBailout_Minigame::~BusinessBailout_Minigame() {
    introduction_timer.clear();
    minigame_timer.clear();
    result_timer.clear();
}

void BusinessBailout_Minigame::start_introduction() {
    update_interval = 500 MILLISECONDS;

    introduction_timer.setInterval([this]() { introduction_update(update_interval); }, update_interval);
}

void BusinessBailout_Minigame::introduction_update(int delta_time) {
    introduction_time -= delta_time;

    if (introduction_time <= 0) {
        introduction_timer.clear();
        start_minigame();
        return;
    }

    send_minigame_introduction(get_camel_case_name(), introduction_time, get_display_name(), get_description());
}

void BusinessBailout_Minigame::start_minigame() {
    for (auto client : game->get_clients()) {
        if (client->isHost) {
            host = client;
        }
        else {
            players.push_back(client);
            player_bail_values[client] = 0;
            player_bail_times[client] = 0;
        }
    }

    minigame_start_time = std::chrono::system_clock::now().time_since_epoch().count();
    minigame_timer.setInterval([this]() {
        update(dt);
    }, dt);
}

void BusinessBailout_Minigame::start_result() {
    result_timer.setTimeout([this]() {
        finished();
    }, result_time);
    flatbuffers::FlatBufferBuilder builder;
    std::vector<flatbuffers::Offset<FBBusinessBailoutResultPair>> result_pairs;
    for (auto player: getMinigameResult()) {
        auto name = builder.CreateString(player->name.c_str());
        result_pairs.push_back(CreateFBBusinessBailoutResultPair(builder, name, player_bail_values[player], player_bail_times[player]));
    }
    auto result_vector = builder.CreateVector(result_pairs);
    auto payload = CreateBusinessBailoutResultPayload(builder, result_vector);
    auto gameStatePayload = CreateMiniGamePayloadType(builder, builder.CreateString(get_camel_case_name()), GameStateType_BusinessBailoutResult,
                                                      GameStatePayload_BusinessBailoutResultPayload, payload.Union());
    game->party->send_gamestate([&](Client *client) {
        return client->client_id == host->client_id ||
               std::find(players.begin(), players.end(), client) != players.end();
    }, builder, gameStatePayload.Union());
}

void BusinessBailout_Minigame::update_value() {
    const float old_value = value;
    const float rnd = static_cast <float> (rand()) / static_cast <float> (RAND_MAX); // generate number, 0 <= x < 1.0
    float change_percent = 2 * volatility * rnd;
    if (change_percent > volatility)
        change_percent -= (2 * volatility);
    const float change_amount = old_value * change_percent;
    value += change_amount;
    if (value < 1) {
        value = 1;
    }
}

void BusinessBailout_Minigame::update(int delta_time) {
    if (time >= minigame_time) {
        minigame_timer.clear();
        start_result();
    }
    time += delta_time;

    update_value();

    send_host_data();
    for (auto player: players) {
        if (player_bail_values[player] == 0) {
            send_player_data(player);
        }
    }
}

void BusinessBailout_Minigame::send_host_data() {
    flatbuffers::FlatBufferBuilder builder;
    std::vector<flatbuffers::Offset<PlayerBailout>> player_bailout_buffer;
    for (auto& [player, bail_value] : player_bail_values) {
        auto name = builder.CreateString(player->name.c_str());
        player_bailout_buffer.push_back(CreatePlayerBailout(builder, name, bail_value, player_bail_times[player]));
    }
    auto player_bailouts = builder.CreateVector(player_bailout_buffer);
    auto payload = CreateBusinessBailoutHostPayload(builder, value, time, player_bailouts);

    auto miniGame = builder.CreateString(get_camel_case_name());
    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_BusinessBailoutHost,
                                                      GameStatePayload_BusinessBailoutHostPayload, payload.Union());
    game->party->send_gamestate([&](Client *client) {
        return client->isHost;
    }, builder, gameStatePayload.Union());
}

void BusinessBailout_Minigame::send_player_data(Client* player) {
    flatbuffers::FlatBufferBuilder builder;
    auto payload = CreateBusinessBailoutPlayerPayload(builder, value, player_bail_values[player] != 0);
    auto gamestatePayload = CreateMiniGamePayloadType(builder, builder.CreateString(get_camel_case_name()), GameStateType_BusinessBailoutPlayer,
                                                      GameStatePayload_BusinessBailoutPlayerPayload, payload.Union());
    game->party->send_gamestate([&](Client *client) {
        return client->client_id == player->client_id;
    }, builder, gamestatePayload.Union());
}

std::vector<Client *> BusinessBailout_Minigame::getMinigameResult() {
    sort(players.begin(), players.end(), [&](Client *a, Client *b) {
        if (player_bail_values[a] == player_bail_values[b]) return rand() % 2 == 0;
        return player_bail_values[a] > player_bail_values[b];
    });
    return players;
}

void BusinessBailout_Minigame::process_input(const MiniGamePayloadType *payload, Client *from) {
    switch(payload->gamestatetype()) {
        case GameStateType_BusinessBailoutPlayerInput:
            player_bail_values[from] = value;
            player_bail_times[from] = std::chrono::system_clock::now().time_since_epoch().count() - minigame_start_time;
            send_player_data(from);
            break;
    }
}