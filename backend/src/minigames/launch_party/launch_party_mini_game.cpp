//
// Created by itssiem on 5/31/24.
//

#include <random>
#include "launch_party_mini_game.h"
#include "../../game.h"

LaunchParty_Minigame::LaunchParty_Minigame(Game *game) : MiniGame(game) {
    std::vector<Client*> clients;
    for (auto client : game->get_clients()) {
        if (client->isHost) {
            host = client;
        }
    }
}

LaunchParty_Minigame::~LaunchParty_Minigame() {
    introduction_timer.clear();
    minigame_timer.clear();
    result_timer.clear();
    timer.clear();
}

void LaunchParty_Minigame::start_introduction() {
    update_interval = 500 MILLISECONDS;
    introduction_timer.setInterval([this]() { introduction_update(update_interval); }, update_interval);
}

void LaunchParty_Minigame::pause() {
    minigame_timer.pause();
    result_timer.pause();
    timer.pause();
    minigame_timer.pause();
}

void LaunchParty_Minigame::resume() {
    minigame_timer.resume();
    result_timer.resume();
    timer.resume();
    minigame_timer.resume();
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
    lights_time = 4 SECONDS;
    wait_time = 5 SECONDS;
    result_time = 10 SECONDS;
    wait_for_green_time = 1500 + (std::rand() % (4000 - 1500 + 1));

    if (practice_round) {
        for (auto client : game->get_clients()) {
            if (!client->isHost) {
                players[client].reaction_time = -10000;
            }
        }
    } else {
        // resets the player ui
        for (auto &player : players) {
            player.second.reaction_time = -10000;
            send_player_data(player.first);
        }
    }

    minigame_timer.setInterval([this]() { update(100 MILLISECONDS); }, 100 MILLISECONDS);
}

void LaunchParty_Minigame::update(int delta_time) {
    if (phase == 0) {
        lights_time -= delta_time;

        send_lights_data(host->client_id, (4 SECONDS - lights_time) / 1000);
        for (auto &player : players) {
            send_lights_data(player.first->client_id, (4 SECONDS - lights_time) / 1000);
        }

        if (lights_time <= 0) {
            phase = 1;
            reaction_time = std::chrono::steady_clock::now();
            send_lights_data(host->client_id, 4);
            for (auto &player : players) {
                send_lights_data(player.first->client_id, 4);
            }
        }
    } else if (phase == 1) {
        wait_for_green_time -= delta_time;
        send_lights_data(host->client_id, 4);
        for (auto &player : players) {
            send_lights_data(player.first->client_id, 4);
        }

        if (wait_for_green_time <= 0) {
            send_lights_data(host->client_id, 5);
            for (auto &player : players) {
                send_lights_data(player.first->client_id, 5);
            }
            reaction_time = std::chrono::steady_clock::now();
            phase = 2;
        }
    } else if (phase == 2) {
        wait_time -= delta_time;

        if (wait_time <= 0) {
            for (auto &player : players) {
                if (player.second.reaction_time == -10000) {
                    player.second.reaction_time = 10 SECONDS;
                }
            }

            minigame_timer.clear();
            phase = 0;
            start_result();
        }
    }
}

void LaunchParty_Minigame::start_result() {

    send_result_data(host->client_id);
    for (auto &player : players) {
        send_result_data(player.first->client_id);
    }

    if (practice_round) {
        result_timer.setTimeout([this]() {
            practice_round = false;
            start_minigame();
        }, 5 SECONDS);
    } else {
        result_timer.setTimeout([this]() {
            finished();
        }, result_time);
    }
}

void LaunchParty_Minigame::process_input(const MiniGamePayloadType *payload, Client *from) {
    if (players[from].reaction_time != -10000) {
        return;
    }
    switch(payload->gamestatetype()) {
        case GameStateType_LaunchPartyPlayerInput: {
            auto input = payload->gamestatepayload_as_LaunchPartyPlayerInputPayload();

            // to calculate the network delay
            if (phase == 1 && !input->pressed()) {
                auto t_end = std::chrono::steady_clock::now();
                players[from].lag_time = std::chrono::duration<double, std::milli>(t_end - reaction_time).count();
            }

            if (input->pressed()) {
                if (phase == 0 || phase == 1) {
                    players[from].reaction_time = -wait_for_green_time;
                    send_player_data(from);
                    players[from].reaction_time = 5 SECONDS + wait_for_green_time;
                }
                if (phase == 2) {
                    auto t_end = std::chrono::steady_clock::now();
                    int difference_ms = std::chrono::duration<double, std::milli>(t_end - reaction_time).count();

                    players[from].reaction_time = difference_ms - players[from].lag_time;
                    send_player_data(from);
                }
            }
        }
    }
}

void LaunchParty_Minigame::send_lights_data(int client_id, int lights_on) {
    flatbuffers::FlatBufferBuilder builder;

    auto payload = CreateLaunchPartyLightsPayload(builder, practice_round, lights_on);

    auto miniGame = builder.CreateString(get_camel_case_name());
    
    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_LaunchPartyLights,
                                                      GameStatePayload_LaunchPartyLightsPayload, payload.Union());

    game->party->send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, builder, gameStatePayload.Union());
}

void LaunchParty_Minigame::send_player_data(Client *player) {
    flatbuffers::FlatBufferBuilder builder;

    int time = players[player].reaction_time;

    auto payload = CreateLaunchPartyPlayerTimePayload(builder, time);

    auto miniGame = builder.CreateString(get_camel_case_name());
    
    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_LaunchPartyPlayerTime,
                                                      GameStatePayload_LaunchPartyPlayerTimePayload, payload.Union());

    game->party->send_gamestate([player](Client* client) { return client->client_id == player->client_id; }, builder, gameStatePayload.Union());
}

void LaunchParty_Minigame::send_result_data(int client_id) {
    flatbuffers::FlatBufferBuilder builder;

    std::vector<Client*> playerResult = getMinigameResult();    

    std::vector<flatbuffers::Offset<FBLaunchPartyResultPair>> results;
    for (auto &player : playerResult) {
        auto result = CreateFBLaunchPartyResultPair(builder, builder.CreateString(player->name), players[player].reaction_time);
        results.push_back(result);
    }
    auto resultsPayload = builder.CreateVector(results);

    auto payload = CreateLaunchPartyResultPayload(builder, resultsPayload);

    auto miniGame = builder.CreateString(get_camel_case_name());
    
    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_LaunchPartyResult,
                                                      GameStatePayload_LaunchPartyResultPayload, payload.Union());

    game->party->send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, builder, gameStatePayload.Union());
}


std::vector<Client *> LaunchParty_Minigame::getMinigameResult() {
    std::vector<Client*> local_players;
    for (auto &player : this->players) {
        local_players.push_back(player.first);
    }

    sort(local_players.begin(), local_players.end(), [&](Client *a, Client *b) {
        if (players[a].reaction_time == players[b].reaction_time)  {
            return true;
        }
        return players[a].reaction_time < players[b].reaction_time;
    });
    return local_players;
}

