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
}

void BusinessBailout_Minigame::update_value() {

    const int n = 25;
    const int a_min = 1;
    const int a_max = 10;
    const int b_min = 1;
    const int b_max = 10;
    const int c_min = 1;
    const int c_max = 10;
    for (int i = 0; i < n; ++i) {
        const int a = rand() % a_max + a_min;
        const int b = rand() % b_max + b_min;
        const int c = (rand() % c_max + c_min) * (i + 1);
        value += sin(time * a + b) * c;
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
        if (player_bail_times[player] == 0) {
            send_player_data(player);
        }
    }
}

void BusinessBailout_Minigame::send_host_data() {
    flatbuffers::FlatBufferBuilder builder;
    std::vector<flatbuffers::Offset<PlayerBailout>> player_bailout_buffer;
    for (auto& [player, bail_time] : player_bail_times) {
        auto name = builder.CreateString(player->name.c_str());
        player_bailout_buffer.push_back(CreatePlayerBailout(builder, name, bail_time));
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
    auto payload = CreateBusinessBailoutPlayerPayload(builder, value, player_bail_times[player] != 0);
    auto gamestatePayload = CreateMiniGamePayloadType(builder, builder.CreateString(get_camel_case_name()), GameStateType_BusinessBailoutPlayer,
                                                      GameStatePayload_BusinessBailoutPlayerPayload, payload.Union());
    game->party->send_gamestate([&](Client *client) {
        return client->client_id == player->client_id;
    }, builder, gamestatePayload.Union());
}

std::vector<Client *> BusinessBailout_Minigame::getMinigameResult() {
    sort(players.begin(), players.end(), [&](Client *a, Client *b) {
        if (player_bail_times[a] == player_bail_times[b]) return rand() % 2 == 0;
        return player_bail_times[a] > player_bail_times[b];
    });
    return players;
}

void BusinessBailout_Minigame::process_input(const MiniGamePayloadType *payload, Client *from) {
    switch(payload->gamestatetype()) {
        case GameStateType_BusinessBailoutPlayerInput:
            player_bail_times[from] = time;
            send_player_data(from);
            break;
    }
}