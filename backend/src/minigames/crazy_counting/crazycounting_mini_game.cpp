//
// Created by itssiem on 5/14/24.
//

#include "crazycounting_mini_game.h"
#include "../../game.h"

CrazyCounting_MiniGame::CrazyCounting_MiniGame(int entity_count, Game* game) : MiniGame(game) {
    update_interval = 16 MILLISECONDS;
    remaining_time = 30 SECONDS;
    time_since_last_time_update = 0 MILLISECONDS;
    this->entity_count = entity_count;
}

void CrazyCounting_MiniGame::start() {
    std::cout << "CrazyCounting_MiniGame started" << std::endl;

    for (Client* client : game->get_clients()) {
        players[client->client_id] = CrazyCounting_Player(client->client_id, entity_count);
    }

    for (int i = 0; i < entity_count; i++) {
        entities.emplace_back();
    }
    timer.startUpdateTimer(this);
}

void CrazyCounting_MiniGame::send_entities() {
    // Create the flatbuffer object
    flatbuffers::FlatBufferBuilder builder;
    std::vector<flatbuffers::Offset<FBCrazyCountingEntity>> entities_buffer;
    for (CrazyCounting_Entity entity: entities) {
        entities_buffer.push_back(CreateFBCrazyCountingEntity(builder, entity.position.x, entity.position.y));
    }
    auto entities_vector = builder.CreateVector(entities_buffer);

    // Encode payload to binary
    auto payload = CreateCrazyCountingHostEntitiesPayload(builder, entities_vector);

    auto gameStatePayload = CreateMiniGamePayloadType(builder, GameStateType_CrazyCountingHostEntities,
                                                      GameStatePayload_CrazyCountingHostEntitiesPayload, payload.Union());

    // Send payload to client
    game->party->send_gamestate([](Client* client) { return client->party->host == client; }, builder, gameStatePayload.Union());
}

void CrazyCounting_MiniGame::send_players_update() {
    for (Client* client: game->get_clients()) {
        if (!client->isHost) {
            send_player_update(client->client_id);
        }
    }
}

void CrazyCounting_MiniGame::send_player_update(int client_id) {
    flatbuffers::FlatBufferBuilder builder;

    auto payload = CreateCrazyCountingPlayerUpdatePayload(builder, players[client_id].get_count(), remaining_time);
    
    auto gameStatePayload = CreateMiniGamePayloadType(builder, GameStateType_CrazyCountingPlayerUpdate,
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

    if(remaining_time <= 0) {
        timer.stop();
        MiniGame::finished();
        return;
    }

    if (time_since_last_time_update >= 1000) {
        time_since_last_time_update = 0;
        send_players_update();
    }

    for (CrazyCounting_Entity& entity: entities) {
        entity.update(delta_time);
    }
    send_entities();
}

std::vector<Client *> CrazyCounting_MiniGame::getMinigameResult() {
    // sort players by count, submitted and time
    std::vector<CrazyCounting_Player*> sorted_players;
    for (auto& [_, player] : players) {
        sorted_players.push_back(&player);
    }
    std::sort(sorted_players.begin(), sorted_players.end());

    std::vector<Client*> result;
    for (CrazyCounting_Player* player: sorted_players) {
        result.push_back(game->get_clients()[player->client_id]);
    }

    return result;
}
