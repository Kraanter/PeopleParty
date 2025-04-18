#include "highway_hustle_mini_game.h"

HighwayHustle_MiniGame::HighwayHustle_MiniGame(Game *game) : MiniGame(game) {
    map = new HighwayHustle_Map(750, 500);
}

HighwayHustle_MiniGame::~HighwayHustle_MiniGame() {
    introduction_timer.clear();
    minigame_timer.clear();
    result_timer.clear();
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
}

void HighwayHustle_MiniGame::resume() {
    minigame_timer.resume();
    result_timer.resume();
    introduction_timer.resume();

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
    map->update(delta_time);
    send_host_update();
    send_players_update();

    // TODO: when time is over, stop timer and start result
    // minigame_timer.clear();
    // start_result();
}

void HighwayHustle_MiniGame::send_host_update()
{
    // Create the flatbuffer object
    flatbuffers::FlatBufferBuilder builder;
    std::vector<flatbuffers::Offset<FBHighwayHustleEntity>> entities_buffer;
    for (auto const& [key, val] : map->players) {
        entities_buffer.push_back(CreateFBHighwayHustleEntity(builder, val->position.x, val->position.y));
    }
    auto entities_vector = builder.CreateVector(entities_buffer);

    std::vector<flatbuffers::Offset<FBHighwayHustleEntity>> obstacles_buffer;
    for (auto const& obstacle : map->obstacles) {
        obstacles_buffer.push_back(CreateFBHighwayHustleEntity(builder, obstacle->position.x, obstacle->position.y));
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

void HighwayHustle_MiniGame::send_players_update()
{
    flatbuffers::FlatBufferBuilder builder;
    // Encode payload to binary
    auto payload = CreateHighwayHustlePlayerPayload(builder, 100);

    auto miniGame = builder.CreateString(get_camel_case_name());

    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_HighwayHustlePlayer,
                                                      GameStatePayload_HighwayHustlePlayerPayload, payload.Union());

    // Send payload to client
    game->party->send_gamestate([](Client* client) { return client->party->host != client; }, builder, gameStatePayload.Union());
}


void HighwayHustle_MiniGame::start_result() {
    // todo: 
    // send_result_data(host->client_id);
    // for (auto &player : players) {
    //     send_result_data(player.first->client_id);
    // }

    // result_timer.setTimeout([this]() {
    //     finished();
    // }, result_time);
}

std::vector<std::pair<Client *, int>> HighwayHustle_MiniGame::getMinigameResult() {
    // std::vector<Client*> local_players;
    // for (auto &player : this->players) {
    //     local_players.push_back(player.first);
    // }

    // sort(local_players.begin(), local_players.end(), [&](Client *a, Client *b) {
    //     if (players[a].total_diff == players[b].total_diff)  {
    //         return true;
    //     }
    //     return players[a].total_diff < players[b].total_diff;
    // });


    // give placement to players (players can have the same placement)
    std::vector<std::pair<Client *, int>> result;
    // for (int i = 0; i < local_players.size(); i++) {
    //     // if the total_diff value of the previous player is the same, give the same placement as previous player
    //     if (i != 0 && players[local_players[i]].total_diff == players[local_players[i - 1]].total_diff ) {
    //         int previous_placement = result[i - 1].second;
    //         result.push_back(std::make_pair(local_players[i], previous_placement));
    //     } else {
    //         result.push_back(std::make_pair(local_players[i], i + 1));
    //     }
    // }
    return result;
}