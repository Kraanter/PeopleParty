//
// Created by itssiem on 5/14/24.
//

#include <random>
#include "crazycounting_mini_game.h"
#include "../../game.h"
#include "../../util/globals.h"

CrazyCounting_MiniGame::CrazyCounting_MiniGame(Game* game) : MiniGame(game) {
    time_since_last_time_update = 0 MILLISECONDS;

    std::mt19937 generator(std::random_device{}());
    std::normal_distribution<double> distribution(distribution_mean, distribution_variance);
    this->entity_count = round(distribution(generator));
    if (entity_count < 1) {
        entity_count = 1;
    }
}

CrazyCounting_MiniGame::~CrazyCounting_MiniGame() {
    introduction_timer.clear();
    timer.clear();
    results_timer.clear();
    for (CrazyCounting_Entity* entity: entities) {
        delete entity;
    }
}

std::string CrazyCounting_MiniGame::get_display_name() {
    return "Crazy Counting";
}

std::string CrazyCounting_MiniGame::get_camel_case_name() {
    return "crazyCounting";
}

std::string CrazyCounting_MiniGame::get_description() {
    return "Count the party hats as fast as you can and fill in the number on your phone!";
}

void CrazyCounting_MiniGame::introduction_update(int delta_time) {
    remaining_time -= delta_time;

    if(remaining_time <= 0) {
        introduction_timer.clear();
        start_minigame();
        return;
    }

    send_minigame_introduction(get_camel_case_name(), remaining_time, get_display_name(), get_description());
}

void CrazyCounting_MiniGame::start_introduction() {
    update_interval = 500 MILLISECONDS;
    remaining_time = introduction_time;

    introduction_timer.setInterval([this]() { introduction_update(update_interval); }, update_interval);
}

void CrazyCounting_MiniGame::pause() {
    timer.pause();
    introduction_timer.pause();
    results_timer.pause();
}

void CrazyCounting_MiniGame::resume() {
    timer.resume();
    introduction_timer.resume();
    results_timer.resume();
}

void CrazyCounting_MiniGame::start_minigame() {
    update_interval = 32 MILLISECONDS;
    remaining_time = 30 SECONDS;

    for (Client* client : game->get_clients()) {
        if (client->isHost) {
            continue;
        }
        players[client->client_id] = CrazyCounting_Player(client->client_id, entity_count);
    }

    for (int i = 0; i < entity_count; i++) {
        entities.push_back(new CrazyCounting_Entity());
    }

    timer.setInterval([this]() { update(update_interval); }, update_interval);
}

void CrazyCounting_MiniGame::start_result() {
    flatbuffers::FlatBufferBuilder builder;

    std::vector<flatbuffers::Offset<FBCrazyCountingResultPair>> results_buffer;
    for (auto& [_, player] : players) {
        auto result = CreateFBCrazyCountingResultPair(builder, 
            builder.CreateString(client_repository[player.client_id]->name), 
            player.get_count());
        
        results_buffer.push_back(result);
    }
    auto results_vector = builder.CreateVector(results_buffer);

    auto payload = CreateCrazyCountingResultPayload(builder, entity_count, results_vector);

    auto miniGame = builder.CreateString("crazyCounting");
    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_CrazyCountingResult,
                                                      GameStatePayload_CrazyCountingResultPayload, payload.Union());

    game->party->send_gamestate([](Client* client) { return client == client; }, builder, gameStatePayload.Union());
}

void CrazyCounting_MiniGame::send_entities() {
    // Create the flatbuffer object
    flatbuffers::FlatBufferBuilder builder;
    std::vector<flatbuffers::Offset<FBCrazyCountingEntity>> entities_buffer;
    for (CrazyCounting_Entity* entity: entities) {
        entities_buffer.push_back(CreateFBCrazyCountingEntity(builder, entity->position.x, entity->position.y));
    }
    auto entities_vector = builder.CreateVector(entities_buffer);

    std::vector<flatbuffers::Offset<flatbuffers::String>> submitted_players;
    for (auto& [_, player] : players) {
        if (player.submitted) {
            submitted_players.push_back(builder.CreateString(client_repository[player.client_id]->name));
        }
    }
    auto submitted_players_vector = builder.CreateVector(submitted_players);

    // Encode payload to binary
    auto payload = CreateCrazyCountingHostEntitiesPayload(builder, remaining_time, entities_vector, submitted_players_vector);

    auto miniGame = builder.CreateString("crazyCounting");

    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_CrazyCountingHostEntities,
                                                      GameStatePayload_CrazyCountingHostEntitiesPayload, payload.Union());

    // Send payload to client
    game->party->send_gamestate([](Client* client) { return client->party->host == client; }, builder, gameStatePayload.Union());
}

void CrazyCounting_MiniGame::send_players_update() {
    for (auto player : players) {
        send_player_update(player.first);
    }
}

void CrazyCounting_MiniGame::send_player_update(int client_id) {
    flatbuffers::FlatBufferBuilder builder;

    auto payload = CreateCrazyCountingPlayerUpdatePayload(builder, players[client_id].get_count(), remaining_time, players[client_id].submitted);

    auto miniGame = builder.CreateString("crazyCounting");
    
    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_CrazyCountingPlayerUpdate,
                                                      GameStatePayload_CrazyCountingPlayerInputPayload, payload.Union());

    game->party->send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, builder, gameStatePayload.Union());
}

void CrazyCounting_MiniGame::process_input(const MiniGamePayloadType* payload, Client* from) {
    switch(payload->gamestatetype()) {
        case GameStateType_CrazyCountingPlayerInput: {
            auto it = players.find(from->client_id);
            if (it == players.end()) {
                players[from->client_id] = CrazyCounting_Player(from->client_id, entity_count);
            }
            CrazyCounting_Player* player = &players[from->client_id];

            auto input = payload->gamestatepayload_as_CrazyCountingPlayerInputPayload();
            switch (input->input_type())
            {
                case Input_Increase: {
                    player->increment_count();
                    send_player_update(from->client_id);
                    break;
                }
                case Input_Decrease: {
                    player->decrement_count();
                    send_player_update(from->client_id);
                    break;
                }
                case Input_Submit: {
                    player->submit();
                    send_player_update(from->client_id);
                    break;
                }
            }
            break;
        }
    }
}

void CrazyCounting_MiniGame::update(int delta_time) {
    remaining_time -= delta_time;
    time_since_last_time_update += delta_time;
    
    // If all the players submitted, end the game
    bool all_submitted = true;
    for (auto& [_, player] : players) {
        if (!player.submitted) {
            all_submitted = false;
            break;
        }
    }
    if (all_submitted) {
        remaining_time = 0;
    }

    if(remaining_time <= 0) {
        timer.clear();
        start_result();

        results_timer.setTimeout([this]() {
            finished();
        }, 5 SECONDS);
        return;
    }

    if (time_since_last_time_update >= 1000) {
        time_since_last_time_update = 0;
        send_players_update();
    }

    for (CrazyCounting_Entity* entity: entities) {
        entity->update(delta_time);
    }
    send_entities();
}

std::vector<std::pair<Client *, int>> CrazyCounting_MiniGame::getMinigameResult() {
    // sort players by count, submitted and time
    std::vector<CrazyCounting_Player> sorted_players;
    for (auto& [_, player] : players) {
        sorted_players.push_back(player);
    }
    std::sort(sorted_players.begin(), sorted_players.end());


    // give placement to players (players can have the same placement)
    std::vector<std::pair<Client *, int>> result;
    for (int i = 0; i < sorted_players.size(); i++) {
        // if the last_changed value of the previous player is the same (and count not), give the same placement as previous player
        if (i != 0 && sorted_players[i].get_count() != sorted_players[i + 1].get_count()
                && sorted_players[i].last_changed == sorted_players[i - 1].last_changed) {
            int previous_placement = result[i - 1].second;
            result.push_back(std::make_pair(client_repository[sorted_players[i].client_id], previous_placement));
        } else {
            result.push_back(std::make_pair(client_repository[sorted_players[i].client_id], i + 1));
        }
    }

    return result;
}

void CrazyCounting_MiniGame::clients_changed(int client_id, bool joined) {
    if (!joined && players.find(client_id) != players.end()) {
        players.erase(client_id);
    }
}
