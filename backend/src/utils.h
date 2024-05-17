//
// Created by itssiem on 5/14/24.
//

#ifndef PEOPLEPARTY_BACKEND_UTILS_H
#define PEOPLEPARTY_BACKEND_UTILS_H

#include <flatbuffers/flatbuffer_builder.h>
#include "flatbuffer/messageClass_generated.h"
#include "globals.h"

void send_message(const std::function<bool(Client*)>& expression, const std::string& message) {
    std::vector<Client*> filtered_clients;
    int client_count = clients.size();
    for (int i = 0; i < client_count; i++) {
        if (expression(clients[i])) {
            filtered_clients.push_back(clients[i]);
        }
    }
    for (Client* client : filtered_clients) {
        client->send(message);
    }
}

void send_gamestate(const std::function<bool(Client*)>& expression, flatbuffers::FlatBufferBuilder& builder, flatbuffers::Offset<> gamestate = 0) {
    auto message = CreateMessage(builder, MessageType_MiniGame, Payload_MiniGamePayloadType, gamestate.Union());

    builder.Finish(message);
    int size = builder.GetSize();
    std::string payload_as_string(reinterpret_cast<const char*>(builder.GetBufferPointer()), size);

    send_message(expression, payload_as_string);
}
#endif //PEOPLEPARTY_BACKEND_UTILS_H
