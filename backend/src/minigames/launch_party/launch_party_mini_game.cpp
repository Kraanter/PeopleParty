//
// Created by itssiem on 5/31/24.
//

#include <random>
#include "launch_party_mini_game.h"
#include "../../game.h"

LaunchParty_Minigame::LaunchParty_Minigame(Game *game) : MiniGame(game) {
    wait_for_green_time = 1000 + (std::rand() % (3000 - 1000 + 1));
}

void LaunchParty_Minigame::start_introduction() {
    introduction_timer.setInterval([this]() { introduction_update(introduction_time); }, introduction_time);
}

void LaunchParty_Minigame::introduction_update(int delta_time) {
    introduction_time -= delta_time;

    if (introduction_time <= 0) {
        introduction_timer.clear();
        start_minigame();
        return;
    }

    send_minigame_introduction(get_camel_case_name(), introduction_time, get_display_name(), get_description());
}

void LaunchParty_Minigame::start_minigame() {
    for (auto client : game->get_clients()) {
        players[client] = -10000;
    }

    minigame_timer.setInterval([this]() { update(100 MILLISECONDS); }, 100 MILLISECONDS);
}

void LaunchParty_Minigame::update(int delta_time) {
    if (phase == 0) {
        lights_time -= delta_time;

        send_lights_data((4 SECONDS - lights_time) / 1000);

        if (lights_time <= 0) {
            phase = 1;
        }
    } else if (phase == 1) {
        wait_for_green_time -= delta_time;

        send_lights_data(5);

        if (wait_for_green_time <= 0) {
            phase = 2;
        }
    } else if (phase == 2) {
        wait_time -= delta_time;

        if (wait_time <= 0) {
            for (auto &player : players) {
                if (player.second == -10000) {
                    player.second = 10 SECONDS;
                }
            }

            phase = 0;
            start_result();
        }

    }
}

void LaunchParty_Minigame::start_result() {
    // todo send results

    result_timer.setTimeout([this]() {
        finished();
    }, result_time);
}

void LaunchParty_Minigame::process_input(const MiniGamePayloadType *payload, Client *from) {
    if (players[from] != -10000) {
        return;
    }
    switch(payload->gamestatetype()) {
        case GameStateType_LaunchPartyPlayerInput: {
            auto input = payload->gamestatepayload_as_LaunchPartyPlayerInputPayload();

            if (input->pressed()) {
                if (phase == 0 || phase == 1) {
                    players[from] = -wait_for_green_time;
                    send_player_data(from);
                    players[from] = 5 SECONDS + wait_for_green_time;
                }
                if (phase == 2) {
                    players[from] = 5 SECONDS - wait_time;
                    send_player_data(from);
                }
            }
        }
    }
}

void LaunchParty_Minigame::send_lights_data(int lights_on) {
    flatbuffers::FlatBufferBuilder builder;

    auto payload = CreateLaunchPartyLightsPayload(builder, lights_on);

    auto miniGame = builder.CreateString(get_camel_case_name());
    
    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_LaunchPartyLights,
                                                      GameStatePayload_LaunchPartyLightsPayload, payload.Union());

    game->party->send_gamestate([](Client* client) { return true; }, builder, gameStatePayload.Union());
}

void LaunchParty_Minigame::send_player_data(Client *player) {
    flatbuffers::FlatBufferBuilder builder;

    int time = players[player];

    auto payload = CreateLaunchPartyPlayerTimePayload(builder, time);

    auto miniGame = builder.CreateString(get_camel_case_name());
    
    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_LaunchPartyPlayerTime,
                                                      GameStatePayload_LaunchPartyPlayerTimePayload, payload.Union());

    game->party->send_gamestate([](Client* client) { return true; }, builder, gameStatePayload.Union());
}

void LaunchParty_Minigame::send_result_data() {
    flatbuffers::FlatBufferBuilder builder;

    std::vector<flatbuffers::Offset<FBLaunchPartyResultPair>> results;
    for (auto &player : players) {
        auto result = CreateFBLaunchPartyResultPair(builder, builder.CreateString(player.first->name), player.second);
        results.push_back(result);
    }
    auto resultsPayload = builder.CreateVector(results);

    auto payload = CreateLaunchPartyResultPayload(builder, resultsPayload);

    auto miniGame = builder.CreateString(get_camel_case_name());
    
    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_LaunchPartyResult,
                                                      GameStatePayload_LaunchPartyResultPayload, payload.Union());

    game->party->send_gamestate([](Client* client) { return true; }, builder, gameStatePayload.Union());
}


std::vector<Client *> LaunchParty_Minigame::getMinigameResult() {
    std::vector<Client*> local_players;
    for (auto &player : this->players) {
        local_players.push_back(player.first);
    }

    sort(local_players.begin(), local_players.end(), [&](Client *a, Client *b) {
        if (players[a] == players[b]) return rand() % 2 == 0;
        return players[a] < players[b];
    });
    return local_players;
}

