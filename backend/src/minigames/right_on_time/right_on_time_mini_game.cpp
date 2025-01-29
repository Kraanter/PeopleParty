#include "right_on_time_mini_game.h"
#include "../../game.h"

RightOnTime_Minigame::RightOnTime_Minigame(Game *game) : MiniGame(game) {
    std::vector<Client*> clients;
    for (auto client : game->get_clients()) {
        if (client->isHost) {
            host = client;
        }
    }

    target = new RightOnTime_Minigame_Target();
    target->round_1_target = (10 + (std::rand() % (15 - 10 + 1))) SECONDS; // between 10 and 15 seconds
    target->round_1_fadeout = target->round_1_target - ((4 + (std::rand() % (6 - 4 + 1))) SECONDS); // between 4 and 6 seconds before target

    target->round_2_target = (15 + (std::rand() % (20 - 15 + 1))) SECONDS; // between 15 and 20 seconds
    target->round_2_fadeout = target->round_2_target - ((6 + (std::rand() % (8 - 6 + 1))) SECONDS); // between 6 and 8 seconds before target

    target->round_3_target = (15 + (std::rand() % (20 - 15 + 1))) SECONDS; // between 15 and 20 seconds
    target->round_3_fadeout = target->round_3_target - ((8 + (std::rand() % (10 - 8 + 1))) SECONDS); // between 8 and 10 seconds before target
}

RightOnTime_Minigame::~RightOnTime_Minigame() {
    introduction_timer.clear();
    minigame_timer.clear();
    result_timer.clear();
    timer.clear();
    delete target;
}

void RightOnTime_Minigame::start_introduction() {
    update_interval = 500 MILLISECONDS;
    introduction_timer.setInterval([this]() { introduction_update(update_interval); }, update_interval);
}

void RightOnTime_Minigame::pause() {
    minigame_timer.pause();
    result_timer.pause();
    timer.pause();
    minigame_timer.pause();
}

void RightOnTime_Minigame::resume() {
    minigame_timer.resume();
    result_timer.resume();
    timer.resume();
    minigame_timer.resume();
}

void RightOnTime_Minigame::introduction_update(int delta_time) {
    introduction_time -= delta_time;

    if (introduction_time <= 0) {
        introduction_timer.clear();
        start_minigame();
        return;
    }

    send_minigame_introduction(get_camel_case_name(), introduction_time, get_display_name(), get_description());
}

void RightOnTime_Minigame::start_minigame() {
    time = 0;
    current_round = 1;
    current_phase = 0;
    minigame_timer.setInterval([this]() { update(10 MILLISECONDS); }, 10 MILLISECONDS);
}

void RightOnTime_Minigame::update(int delta_time) {
    time += delta_time;

    if (current_round == 1 && time >= target->round_1_target + 10 SECONDS) {
        if (current_phase == 0) {
            current_phase = 1;
            // send round result
        } else {
            result_time -= delta_time;
            if (result_time <= 0) {
                current_round++;
                current_phase = 0;
                time = 0;
                result_time = 5 SECONDS;
            }
        }
    }
    else if (current_round == 2 && time >= target->round_2_target + 10 SECONDS) {
        if (current_phase == 0) {
            current_phase = 1;
            // send round result
        } else {
            result_time -= delta_time;
            if (result_time <= 0) {
                current_round++;
                current_phase = 0;
                time = 0;
                result_time = 5 SECONDS;
            }
        }
    }
    else if (current_round == 3 && time >= target->round_3_target + 10 SECONDS) {
        if (current_phase == 0) {
            current_phase = 1;
            // send round result
        } else {
            result_time -= delta_time;
            if (result_time <= 0) {
                current_round++;
                current_phase = 0;
                time = 0;
                result_time = 5 SECONDS;
            }
        }
    }
    else if (current_round == 4) {
        minigame_timer.clear();
        start_result();
    }
    else {
        // send minigame data
        int round_target = current_round == 1 ? target->round_1_target : current_round == 2 ? target->round_2_target : target->round_3_target;
        bool round_fadeout = current_round == 1 && time > target->round_1_fadeout ? true : current_round == 2 && time > target->round_2_fadeout ? true : current_round == 3 && time > target->round_3_fadeout ? true : false;

        send_payload_data(host->client_id, false, round_target, round_fadeout);
        for (auto &player : players) {
            send_payload_data(player.first->client_id, true, round_target, round_fadeout);
        }
    }
}

void RightOnTime_Minigame::process_input(const MiniGamePayloadType *payload, Client *from) {
    switch(payload->gamestatetype()) {
        case GameStateType_RightOnTime: {
            auto input = payload->gamestatepayload_as_RightOnTimePayload();

            if (current_phase == 0) {
                if (current_round == 1) {
                    players[from].round_1_diff = input->time();
                }
                else if (current_round == 2) {
                    players[from].round_2_diff = input->time();
                }
                else if (current_round == 3) {
                    players[from].round_3_diff = input->time();
                    players[from].total_diff = abs(players[from].round_1_diff) + abs(players[from].round_2_diff) + abs(players[from].round_3_diff);
                }
            }
        }
    }
}

void RightOnTime_Minigame::send_payload_data(int client_id, bool is_host, int round_target, bool round_fadeout) {
    flatbuffers::FlatBufferBuilder builder;

    std::vector<flatbuffers::Offset<flatbuffers::String>> submitted_players;
    if (is_host) {
        for (auto &player : players) {
            if (current_round == 1 && player.second.round_1_diff != 0 ? true : current_round == 2 && player.second.round_2_diff != 0 ? true : current_round == 3 && player.second.round_3_diff != 0 ? true : false) {
                submitted_players.push_back(builder.CreateString(player.first->name));
            }
        }
    }  
    auto submitted_players_vector = builder.CreateVector(submitted_players);

    auto payload = CreateRightOnTimePayload(builder, current_round, round_target, time, round_fadeout, submitted_players_vector);

    auto miniGame = builder.CreateString(get_camel_case_name());
    
    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_RightOnTime,
                                                      GameStatePayload_RightOnTimePayload, payload.Union());

    game->party->send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, builder, gameStatePayload.Union());
}

void RightOnTime_Minigame::send_round_result_data(int client_id) {
    flatbuffers::FlatBufferBuilder builder;

    int round_target = current_round == 1 ? target->round_1_target : current_round == 2 ? target->round_2_target : target->round_3_target;

    std::vector<flatbuffers::Offset<FBRightOnTimeRoundResultPair>> results;
    for (auto &player : players) {
        int round_diff = current_round == 1 ? player.second.round_1_diff : current_round == 2 ? player.second.round_2_diff : player.second.round_3_diff;
        auto result = CreateFBRightOnTimeRoundResultPair(builder, builder.CreateString(player.first->name), round_diff);
        results.push_back(result);
    }

    auto resultsPayload = builder.CreateVector(results);

    auto payload = CreateRightOnTimeRoundResultPayload(builder, current_round, round_target, resultsPayload);

    auto miniGame = builder.CreateString(get_camel_case_name());
    
    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_RightOnTimeRoundResult,
                                                      GameStatePayload_RightOnTimeRoundResultPayload, payload.Union());

    game->party->send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, builder, gameStatePayload.Union());
}

void RightOnTime_Minigame::start_result() {
    send_result_data(host->client_id);
    for (auto &player : players) {
        send_result_data(player.first->client_id);
    }

    result_timer.setTimeout([this]() {
        finished();
    }, result_time);
}

void RightOnTime_Minigame::send_result_data(int client_id) {
    flatbuffers::FlatBufferBuilder builder;

    std::vector<Client*> playerResult = getMinigameResult();    

    std::vector<flatbuffers::Offset<FBRightOnTimeResultPair>> results;
    for (auto &player : playerResult) {
        auto result = CreateFBRightOnTimeResultPair(builder, builder.CreateString(player->name), players[player].round_1_diff, players[player].round_2_diff, players[player].round_3_diff, players[player].total_diff);
        results.push_back(result);
    }
    auto resultsPayload = builder.CreateVector(results);

    auto payload = CreateRightOnTimeResultPayload(builder, resultsPayload);

    auto miniGame = builder.CreateString(get_camel_case_name());
    
    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_RightOnTimeResult,
                                                      GameStatePayload_RightOnTimeResultPayload, payload.Union());

    game->party->send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, builder, gameStatePayload.Union());
}

std::vector<Client *> RightOnTime_Minigame::getMinigameResult() {
    std::vector<Client*> local_players;
    for (auto &player : this->players) {
        local_players.push_back(player.first);
    }

    sort(local_players.begin(), local_players.end(), [&](Client *a, Client *b) {
        if (players[a].total_diff == players[b].total_diff)  {
            return true;
        }
        return players[a].total_diff < players[b].total_diff;
    });
    return local_players;
}