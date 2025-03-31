//
// Created by itssiem on 5/31/24.
//

#include <random>
#include "business_bailout_minigame.h"
#include "../../game.h"

BusinessBailout_Minigame::BusinessBailout_Minigame(Game *game) : MiniGame(game) {
    minigame_time = min_duration + (std::rand() % (max_duration - min_duration + 1));
    minigame_time = minigame_time / 1000 * 1000;

    int points = minigame_time / dt;
    double value = 0;
    double upward_bias = 1.15;
    double max_step_up = 20.0;
    double min_step_up = 2.0;
    double max_step_down = 25.0;
    double min_step_down = 3.0;

    int direction = 1; // 1 = up, -1 = down
    int same_direction_steps = 0;

    // ensures it doesnt oscillate too much
    int min_steps_same_direction = 6;
    int max_steps_same_direction = 25;

    same_direction_steps = min_steps_same_direction + (std::rand() % (max_steps_same_direction - min_steps_same_direction));

    for (int i = 0; i < points; ++i) {
        double step;

        if (direction == 1) {
            step = min_step_up + (std::rand() % static_cast<int>(max_step_up - min_step_up));
            step += upward_bias * (static_cast<double>(i) / points);
        } else {
            step = min_step_down + (std::rand() % static_cast<int>(max_step_down - min_step_down));
            step *= 1.0 + (std::rand() % 30) / 100.0; // Add slight randomness to downward steps
        }

        value += step * direction;

        if (value < 0) {
            value = 0;
            direction = 1;
            same_direction_steps = min_steps_same_direction;
        }

        path.push_back(value);

        same_direction_steps--;

        if (same_direction_steps <= 0) {
            // Bias towards upward movement, but allow more frequent dips
            if (direction == -1 || std::rand() % 100 < 60) {
                direction = 1;
            } else {
                direction = -1;
            }

            same_direction_steps = min_steps_same_direction + (std::rand() % (max_steps_same_direction - min_steps_same_direction));
        }
    }

    path.push_back(0);
    path.push_back(0);
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

void BusinessBailout_Minigame::pause() {
    timer.pause();
    introduction_timer.pause();
    minigame_timer.pause();
    result_timer.pause();
}

void BusinessBailout_Minigame::resume() {
    timer.resume();
    introduction_timer.resume();
    minigame_timer.resume();
    result_timer.resume();
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

    minigame_start_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
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
    value = path[time / dt];
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
        return player_bail_values[a] > player_bail_values[b];
    });
    return players;
}

void BusinessBailout_Minigame::process_input(const MiniGamePayloadType *payload, Client *from) {
    switch(payload->gamestatetype()) {
        case GameStateType_BusinessBailoutPlayerInput:
            player_bail_values[from] = value;
            player_bail_times[from] = time;
            send_player_data(from);
            break;
    }
}