//
// Created by itssiem on 5/14/24.
//

#include "crazycounting_mini_game.h"
#include "../../utils.h"

CrazyCounting_MiniGame::CrazyCounting_MiniGame(int entity_count, Game* game) : MiniGame(game) {
    update_interval = 16 MILLISECONDS;
    remaining_time = 30 SECONDS;
    time_since_last_time_update = 0 MILLISECONDS;
    this->entity_count = entity_count;
}

void CrazyCounting_MiniGame::start() {
    std::cout << "CrazyCounting_MiniGame started" << std::endl;

    for (Client* client : game->clients) {
        players[client->client_id] = CrazyCounting_Player(client->client_id);
    }

    for (int i = 0; i < entity_count; i++) {
        entities.emplace_back();
    }
    timer.startUpdateTimer(this);
//    update(delta_time);
}

void CrazyCounting_MiniGame::send_entities() {
    // Create the flatbuffer object
    flatbuffers::FlatBufferBuilder builder;
    std::vector<flatbuffers::Offset<FBCrazyCountingEntity>> entities_buffer;
    for (CrazyCounting_Entity entity: entities) {
        entities_buffer.push_back(CreateFBCrazyCountingEntity(builder, entity.position.first, entity.position.second));
    }
    auto entities_vector = builder.CreateVector(entities_buffer);

    // Encode payload to binary
    auto payload = CreateCrazyCountingHostEntitiesPayload(builder, entities_vector);

    auto gameStatePayload = CreateMiniGamePayloadType(builder, GameStateType_CrazyCountingHostEntities,
                                                      GameStatePayload_CrazyCountingHostEntitiesPayload, payload.Union());

    // Send payload to client
    send_gamestate([](Client* client) { return client->party->host == client; }, builder, gameStatePayload.Union());
}

void CrazyCounting_MiniGame::send_players_update() {
    for (Client* client: game->clients) {
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

    send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, builder, gameStatePayload.Union());
}

void CrazyCounting_MiniGame::process_input(const MiniGamePayloadType* payload, Client* from) {
    switch(payload->gamestatetype()) {
        case GameStateType_CrazyCountingPlayerInput: {
            auto it = players.find(from->client_id);
            if (it == players.end()) {
                players[from->client_id] = CrazyCounting_Player(from->client_id);
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

    for (CrazyCounting_Entity entity: entities) {
        entity.update(delta_time);
    }
    send_entities();
}
