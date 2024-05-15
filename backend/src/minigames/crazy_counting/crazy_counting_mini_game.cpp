//
// Created by itssiem on 5/14/24.
//

#include "crazy_counting_mini_game.h"
#include "../../flatbuffer/messageClass_generated.h"
#include "../../utils.h"
#include <cmath>

CrazyCountingMiniGame::CrazyCountingMiniGame(int entity_count, const Party& party) : Minigame(party) {
    for (int i = 0; i < entity_count; i++) {
        entities.emplace_back();
    }
}

void CrazyCountingMiniGame::send_entities() {
    // Create the flatbuffer object
    flatbuffers::FlatBufferBuilder builder;
    std::vector<flatbuffers::Offset<FBCrazyCountingEntity>> entities_buffer;
    for (Entity entity: entities) {
        entities_buffer.push_back(CreateFBCrazyCountingEntity(builder, entity.position.first, entity.position.second));
    }
    auto entities_vector = builder.CreateVector(entities_buffer);

    // Encode payload to binary
    auto payload = CreateCrazyCountingHostEntitiesPayload(builder, entities_vector);

    // Send payload to client
    send_gamestate([](Client* client) { return client->party->host == client; }, builder, payload.Union());
}

void CrazyCountingMiniGame::send_count(int client_id) {
    // todo: Create the flatbuffer payload


    // todo: Encode payload to binary

    // todo: Send payload to client
}

void CrazyCountingMiniGame::process_input(const std::string &payload) {
    // todo: Parse payload

    // todo: Increment or Decrement the counter

    // todo: Send new count back to client
}

void CrazyCountingMiniGame::update(unsigned long delta_time) {
    for (Entity entity: entities) {
        entity.update(delta_time);
    }
    send_entities();
}
