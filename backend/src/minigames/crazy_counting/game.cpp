//
// Created by itssiem on 5/14/24.
//

#include "game.h"
#include <cmath>

CrazyCountingGame::CrazyCountingGame(int entity_count, const Party& party) : Minigame(party) {
    for (int i = 0; i < entity_count; i++) {
        entities.emplace_back();
    }
}

void CrazyCountingGame::send_entities() {
    // todo: Create the flatbuffer payload

    // todo: Encode payload to binary

    // todo: Send payload to client
}

void CrazyCountingGame::send_count(int client_id) {
    // todo: Create the flatbuffer payload

    // todo: Encode payload to binary

    // todo: Send payload to client
}

void CrazyCountingGame::process_input(const std::string &payload) {
    // todo: Parse payload

    // todo: Increment or Decrement the counter

    // todo: Send new count back to client
}

void CrazyCountingGame::update(unsigned long delta_time) {
    for (Entity entity: entities) {
        entity.update(delta_time);
    }
}
