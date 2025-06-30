#include "highway_hustle_mini_game.h"

HighwayHustle_MiniGame::HighwayHustle_MiniGame(Game *game) : MiniGame(game) {
    map = new HighwayHustle_Map(750, 480);
}

HighwayHustle_MiniGame::~HighwayHustle_MiniGame() {
    introduction_timer.clear();
    minigame_timer.clear();
    result_timer.clear();
    timer.clear();
    delete map;
}

void HighwayHustle_MiniGame::start_introduction() {
    update_interval = 500 MILLISECONDS;
    introduction_timer.setInterval([this]() { introduction_update(update_interval); }, update_interval);
}

void HighwayHustle_MiniGame::pause() {
    minigame_timer.pause();
    result_timer.pause();
    introduction_timer.pause();
    timer.pause();
}

void HighwayHustle_MiniGame::resume() {
    minigame_timer.resume();
    result_timer.resume();
    introduction_timer.resume();
    timer.resume();
}

void HighwayHustle_MiniGame::introduction_update(int delta_time)
{
    introduction_time -= delta_time;

    if (introduction_time <= 0) {
        introduction_timer.clear();
        start_minigame();
        return;
    }

    send_minigame_introduction(get_camel_case_name(), introduction_time, get_display_name(), get_description());
}

void HighwayHustle_MiniGame::start_minigame() {
    map->create_players(game->get_clients());

    update_interval = floor(1000 / target_fps);

    minigame_timer.setInterval([this]() { update(update_interval); }, update_interval);
}

void HighwayHustle_MiniGame::process_input(const MiniGamePayloadType *payload, Client *from) {
    // decode the payload

    //map->update_player_velocity(from, x, y);
    switch(payload->gamestatetype()) {
        case GameStateType_JoystickData: {
            auto input = payload->gamestatepayload_as_JoystickDataPayload();

            if (input->x_pos() != 0 || input->y_pos() != 0) {
                // invert y axis
                map->update_player_velocity(from, input->x_pos(), -input->y_pos());
            }
            break;
        }
        case GameStateType_JoystickEvent: {
            auto input = payload->gamestatepayload_as_JoystickEventPayload();

            switch (input->event_type())
            {
                case JoystickEventType_Start: {
                    // start moving
                    map->update_player_event(from, 1);
                    break;
                }
                case JoystickEventType_Stop: {
                    // stop moving
                    map->update_player_event(from, 0);
                    break;
                }
                break;
            }
        }
        break;
    }
}

void HighwayHustle_MiniGame::update(int delta_time) {
    // check if minigame ended
    if (!map->check_players_alive()) {
        minigame_timer.clear();
        timer.setTimeout([this]() {
            start_result();
        }, 1000);
        return;
    }

    map->update(delta_time);
    send_host_update();
    for (auto const& [key, val] : map->players) {
        send_player_update(key, val);
    }
}

void HighwayHustle_MiniGame::send_host_update()
{
    // Create the flatbuffer object
    flatbuffers::FlatBufferBuilder builder;
    std::vector<flatbuffers::Offset<FBHighwayHustleEntity>> entities_buffer;
    for (auto const& [key, val] : map->players) {
        auto id = builder.CreateString(key->name);
        entities_buffer.push_back(CreateFBHighwayHustleEntity(builder, id, val->position.x, val->position.y, val->car_type, val->is_dead));
    }
    auto entities_vector = builder.CreateVector(entities_buffer);

    std::vector<flatbuffers::Offset<FBHighwayHustleEntity>> obstacles_buffer;
    for (auto const& obstacle : map->obstacles) {
        auto id = builder.CreateString(obstacle->id);
        obstacles_buffer.push_back(CreateFBHighwayHustleEntity(builder, id, obstacle->position.x, obstacle->position.y, obstacle->car_type, false));
    }
    auto obstacles_vector = builder.CreateVector(obstacles_buffer);

    // Encode payload to binary
    auto payload = CreateHighwayHustleHostPayload(builder, entities_vector, obstacles_vector, map->getDistanceTravelled());

    auto miniGame = builder.CreateString(get_camel_case_name());

    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_HighwayHustleHost,
                                                      GameStatePayload_HighwayHustleHostPayload, payload.Union());

    // Send payload to client
    game->party->send_gamestate([](Client* client) { return client->party->host == client; }, builder, gameStatePayload.Union());
}

void HighwayHustle_MiniGame::send_player_update(Client *client, Moving_Entity *entity) {
    flatbuffers::FlatBufferBuilder builder;
    // Encode payload to binary
    int score = entity->is_dead ? entity->final_score : map->getDistanceTravelled();
    auto payload = CreateHighwayHustlePlayerPayload(builder, score, entity->is_dead, entity->car_type);

    auto miniGame = builder.CreateString(get_camel_case_name());

    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_HighwayHustlePlayer,
                                                      GameStatePayload_HighwayHustlePlayerPayload, payload.Union());

    // Send payload to client
    game->party->send_gamestate([client](Client* c) { return c == client; }, builder, gameStatePayload.Union());
}

void HighwayHustle_MiniGame::start_result() {
    send_result_data(game->party->host->client_id);
    for (auto &player : map->players) {
        send_result_data(player.first->client_id);
    }

    result_timer.setTimeout([this]() {
        finished();
    }, 5 SECONDS);
}

void HighwayHustle_MiniGame::send_result_data(int client_id)
{
    flatbuffers::FlatBufferBuilder builder;
    auto mini_game_result = getMinigameResult();

    std::vector<flatbuffers::Offset<FBHighwayHustleResultPair>> results_buffer;
    for (auto &player: map->players) {
        // get placement from results
        auto placement = 99;
        for (auto &result : mini_game_result) {
            if (result.first == player.first) {
                placement = result.second;
                break;
            }
        }
        // make result pair
        results_buffer.push_back(CreateFBHighwayHustleResultPair(builder, builder.CreateString(player.first->name), player.second->final_score, placement));
    }
    auto results_vector = builder.CreateVector(results_buffer);

    auto payload = CreateHighwayHustleResultPayload(builder, results_vector);

    auto miniGame = builder.CreateString(get_camel_case_name());
    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_HighwayHustleResult,
                                                      GameStatePayload_HighwayHustleResultPayload, payload.Union());

    game->party->send_gamestate([](Client* client) { return client == client; }, builder, gameStatePayload.Union());
}

std::vector<std::pair<Client *, int>> HighwayHustle_MiniGame::getMinigameResult() {
    std::vector<Client*> local_players;
    for (auto &player : this->map->players) {
        local_players.push_back(player.first);
    }

    sort(local_players.begin(), local_players.end(), [&](Client *a, Client *b) {
        if (map->players[a]->final_score == map->players[b]->final_score)  {
            return true;
        }
        return map->players[a]->final_score > map->players[b]->final_score;
    });


    // give placement to players (players can have the same placement)
    std::vector<std::pair<Client *, int>> result;
    for (int i = 0; i < local_players.size(); i++) {
        // if the final_score value of the previous player is the same, give the same placement as previous player
        if (i != 0 && map->players[local_players[i]]->final_score == map->players[local_players[i - 1]]->final_score ) {
            int previous_placement = result[i - 1].second;
            result.push_back(std::make_pair(local_players[i], previous_placement));
        } else {
            result.push_back(std::make_pair(local_players[i], i + 1));
        }
    }
    return result;
}