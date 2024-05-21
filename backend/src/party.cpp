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

void Party::add_client(Client *client) { clients.push_back(client); }

void Party::remove_client(Client *client) {
  for (int i = 0; i < clients.size(); ++i) {
    if (clients[i] == client) {
      clients.erase(clients.begin() + i);
      return;
    }
  }
  std::cerr << "Tried to remove " << *client
            << " but it was not found in party " << party_id << "\n";
}

void Party::promote_client(Client *client) {
  if (std::find(clients.begin(), clients.end(), client) != clients.end()) {
      host->isHost = false;
      client->isHost = true;
      host = client;
  } else {
    std::cerr << "Tried to promote " << *client << " but it was found in party "
              << party_id << "\n";
  }
}

const void Party::send_message(const std::function<bool(Client *)> &expression, const std::string &message) {
    std::vector<Client*> filtered_clients;
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

std::ostream &operator<<(std::ostream &stream, const Party &party) {
  std::string game_name = "Nothing";
  stream << party.party_id << ", playing " << game_name << ", "
         << party.clients.size() << " clients connected";
  return stream;
}

int generate_party_id() {
  const int random = rand();
  int id = (random % 9000) + 1000;
  while (parties.contains(id)) {
    id++;
    if (id > 9999) {
      id = 1000;
    }
  }
  return id;
}
