//
// Created by itssiem on 5/14/24.
//

#include "crazycounting_mini_game.h"
#include "../../flatbuffer/messageClass_generated.h"
#include "../../utils.h"
#include <cmath>

CrazyCounting_MiniGame::CrazyCounting_MiniGame(int entity_count, const Party& party) : Minigame(party) {
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

    // Send payload to client
    send_gamestate([](Client* client) { return client->party->host == client; }, builder, payload.Union());
}

void CrazyCounting_MiniGame::send_count(int client_id) {
    // todo: Create the flatbuffer payload


    // todo: Encode payload to binary

    // todo: Send payload to client
}

void CrazyCounting_MiniGame::process_input(const std::string &payload) {
    // todo: Parse payload

    // todo: Increment or Decrement the counter

    // todo: Send new count back to client
}

void CrazyCounting_MiniGame::update(unsigned long delta_time) {
    for (CrazyCounting_Entity entity: entities) {
        entity.update(delta_time);
    }
    send_entities();
}
