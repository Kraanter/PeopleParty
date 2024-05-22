#include "party.h"

#include "globals.h"
#include "game.h"

Party::Party() {
  party_id = generate_party_id();
  host = nullptr;
  game = nullptr;
}

void Party::start_game() {
    game = new Game(this);
}

const std::vector<Client *> Party::get_clients() {
    return client_repository.Find([&] (Client* client) {
        return client->party->party_id == this->party_id;
    });
}

const void Party::send_message(const std::function<bool(Client *)> &expression, const std::string &message) {
    std::vector<Client*> filtered_clients;
    std::vector<Client*> clients = get_clients();
    int client_count = clients.size();
    for (int i = 0; i < client_count; i++) {
        if (expression(clients[i])) {
            filtered_clients.push_back(clients[i]);
        }
    }
    for (Client* client : filtered_clients) {
        server_loop->defer([client, message]() {
            client->send(message);
        });
    }
}

const void Party::send_gamestate(const std::function<bool(Client *)> &expression, flatbuffers::FlatBufferBuilder &builder,
                           flatbuffers::Offset<> gamestate) {
    auto message = CreateMessage(builder, MessageType_MiniGame, Payload_MiniGamePayloadType, gamestate.Union());

    builder.Finish(message);
    int size = builder.GetSize();

    uint8_t* buffer = builder.GetBufferPointer();
    std::string payload_as_string(reinterpret_cast<const char*>(builder.GetBufferPointer()), size);

    send_message(expression, payload_as_string);
}

const void
Party::send_party_prep(const std::function<bool(Client *)> &expression, flatbuffers::FlatBufferBuilder &builder,
                       flatbuffers::Offset<> partyprep_payload) {
    auto message = CreateMessage(builder, MessageType_PartyPrep, Payload_PartyPrepPayloadType, partyprep_payload.Union());
    builder.Finish(message);
    int size = builder.GetSize();

    std::string payload_as_string(reinterpret_cast<const char*>(builder.GetBufferPointer()), size);

    send_message(expression, payload_as_string);
    return;
}

const void Party::send_leaderboard(const std::function<bool(Client *)> &expression, flatbuffers::FlatBufferBuilder &builder,
                            flatbuffers::Offset<> leaderboard_payload) {
    auto message = CreateMessage(builder, MessageType_Leaderboard, Payload_LeaderboardPayloadType, leaderboard_payload.Union());
    builder.Finish(message);
    int size = builder.GetSize();

    std::string payload_as_string(reinterpret_cast<const char*>(builder.GetBufferPointer()), size);

    send_message(expression, payload_as_string);
    return;
}

const void Party::clients_changed() {
    if (game != nullptr) {
        game->clients_changed();
    }
}

std::ostream &operator<<(std::ostream &stream, Party &party) {
  std::string game_name = "Nothing";
  stream << party.party_id << ", playing " << game_name << ", "
         << party.get_clients().size() << " client_repository connected";
  return stream;
}

int generate_party_id() {
  const int random = rand();
  int id = (random % 9000) + 1000;
  while (party_repository.contains(id)) {
    id++;
    if (id > 9999) {
      id = 1000;
    }
  }
  return id;
}
