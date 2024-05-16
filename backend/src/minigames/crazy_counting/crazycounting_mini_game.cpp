//
// Created by itssiem on 5/14/24.
//

#include "crazycounting_mini_game.h"
#include "../../flatbuffer/messageClass_generated.h"
#include "../../utils.h"
#include <cmath>

CrazyCounting_MiniGame::CrazyCounting_MiniGame(int entity_count, Game* game) : MiniGame(game) {
    for (int i = 0; i < entity_count; i++) {
        entities.emplace_back();
    }
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
    auto payload = CreateCrazyCountingPlayerUpdatePayload(builder, counting_register.get_count(client_id), 1000);
    
    auto gameStatePayload = CreateMiniGamePayloadType(builder, GameStateType_CrazyCountingPlayerUpdate,
                                                      GameStatePayload_CrazyCountingPlayerInputPayload, payload.Union());

    send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, builder, gameStatePayload.Union());
}

void CrazyCounting_MiniGame::process_input(const MiniGamePayloadType* payload, Client* from) {
    switch(payload->gamestatetype()) {
        case GameStateType_CrazyCountingPlayerInput: {
            auto input = payload->gamestatepayload_as_CrazyCountingPlayerInputPayload();
            switch (input->input_type())
            {
                case Input_Increase: {
                    counting_register.increase(from->client_id);
                    send_player_update(from->client_id);
                    break;
                }
                case Input_Decrease: {
                    counting_register.decrease(from->client_id);
                    send_player_update(from->client_id);
                    break;
                }
                case Input_Submit: {
                    // TODO: stop a timer
                    break;
                }
            }
            break;
        }
    }
}

void CrazyCounting_MiniGame::update(unsigned long delta_time) {
    for (CrazyCounting_Entity entity: entities) {
        entity.update(delta_time);
    }
    send_entities();
}
