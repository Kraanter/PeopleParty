#include "unscrambled_mini_game.h"
#include "../../game.h"

Unscrambled_Minigame::Unscrambled_Minigame(Game *game) : MiniGame(game) {

}

Unscrambled_Minigame::~Unscrambled_Minigame() {
    introduction_timer.clear();
    minigame_timer.clear();
    result_timer.clear();
    timer.clear();
}

void Unscrambled_Minigame::start_introduction() {
    update_interval = 500 MILLISECONDS;
    introduction_timer.setInterval([this]() { introduction_update(update_interval); }, update_interval);
}

void Unscrambled_Minigame::pause() {
    minigame_timer.pause();
    result_timer.pause();
    timer.pause();
    introduction_timer.pause();
}

void Unscrambled_Minigame::resume() {
    minigame_timer.resume();
    result_timer.resume();
    timer.resume();
    introduction_timer.resume();
}

void Unscrambled_Minigame::introduction_update(int delta_time) {
    introduction_time -= delta_time;

    if (introduction_time <= 0) {
        introduction_timer.clear();
        start_minigame();
        return;
    }

    send_minigame_introduction(get_camel_case_name(), introduction_time, get_display_name(), get_description());
}

void Unscrambled_Minigame::start_minigame() {
    time = 0;
    current_round = 1;
    current_phase = 0;

    minigame_timer.setInterval([this]() { update(100 MILLISECONDS); }, 100 MILLISECONDS);
}

void Unscrambled_Minigame::update(int delta_time) {
    time += delta_time;

    if (current_round == max_rounds + 1) {
        minigame_timer.clear();
        start_result();
    }
    else {
        // send minigame data
        send_host_payload_data(game->party->host->client_id);
        for (auto &player : players) {
            send_player_payload_data(player.first->client_id);
        }
    }
}

void Unscrambled_Minigame::process_input(const MiniGamePayloadType *payload, Client *from) {
    switch(payload->gamestatetype()) {
        case GameStateType_RightOnTime: {
            // TODO: change to new payload enum and proccess the input accordingly
        }
    }
}

void Unscrambled_Minigame::send_host_payload_data(int client_id) {
    flatbuffers::FlatBufferBuilder builder;

    // auto payload = CreateRightOnTimePayload(builder, current_round, round_target, time, round_fadeout, submitted_players_vector);

    // auto miniGame = builder.CreateString(get_camel_case_name());
    
    // auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_RightOnTime,
    //                                                   GameStatePayload_RightOnTimePayload, payload.Union());

    // game->party->send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, builder, gameStatePayload.Union());
}

void Unscrambled_Minigame::send_player_payload_data(int client_id) {
    flatbuffers::FlatBufferBuilder builder;

    // auto payload = CreateRightOnTimePayload(builder, current_round, round_target, time, round_fadeout, submitted_players_vector);

    // auto miniGame = builder.CreateString(get_camel_case_name());
    
    // auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_RightOnTime,
    //                                                   GameStatePayload_RightOnTimePayload, payload.Union());

    // game->party->send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, builder, gameStatePayload.Union());
}

void Unscrambled_Minigame::send_round_result_data(int client_id) {
    flatbuffers::FlatBufferBuilder builder;

    // auto payload = CreateRightOnTimeRoundResultPayload(builder, current_round, round_target, resultsPayload);

    // auto miniGame = builder.CreateString(get_camel_case_name());
    // auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_RightOnTimeRoundResult,
    //                                                   GameStatePayload_RightOnTimeRoundResultPayload, payload.Union());

    // game->party->send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, builder, gameStatePayload.Union());
}

void Unscrambled_Minigame::start_result() {
    send_result_data(game->party->host->client_id);
    for (auto &player : players) {
        send_result_data(player.first->client_id);
    }

    result_timer.setTimeout([this]() {
        finished();
    }, result_time);
}

void Unscrambled_Minigame::send_result_data(int client_id) {
    flatbuffers::FlatBufferBuilder builder;

    // auto payload = CreateRightOnTimeResultPayload(builder, resultsPayload);

    // auto miniGame = builder.CreateString(get_camel_case_name());
    
    // auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_RightOnTimeResult,
    //                                                   GameStatePayload_RightOnTimeResultPayload, payload.Union());

    // game->party->send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, builder, gameStatePayload.Union());
}

std::vector<std::pair<Client *, int>> Unscrambled_Minigame::getMinigameResult() {
    std::vector<std::pair<Client *, int>> local_players;
    for (auto &player : this->players) {
        local_players.push_back(player);
    }

    sort(local_players.begin(), local_players.end(), [&](const std::pair<Client *, int> &a, const std::pair<Client *, int> &b) {
        if (a.second == b.second)  {
            return true;
        }
        return b.second < a.second;
    });


    // give placement to players (players can have the same placement)
    std::vector<std::pair<Client *, int>> result;
    for (int i = 0; i < local_players.size(); i++) {
        // if the total_diff value of the previous player is the same, give the same placement as previous player
        if (i != 0 && local_players[i].second == local_players[i - 1].second) {
            int previous_placement = result[i - 1].second;
            result.push_back(std::make_pair(local_players[i].first, previous_placement));
        } else {
            result.push_back(std::make_pair(local_players[i].first, i + 1));
        }
    }
    return result;
}