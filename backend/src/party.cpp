#include "party.h"

#include "globals.h"

Party::Party() {
  party_id = generate_party_id();
  host = nullptr;
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

void Party::send(const uint8_t *payload, size_t size, uWS::OpCode opcode) {
  for (Client *client : clients) {
    client->send(payload, size, opcode);
  }
}

void Party::send(std::string payload, uWS::OpCode opcode) {
  for (Client *client : clients) {
    client->send(payload, opcode);
  }
}

std::ostream &operator<<(std::ostream &stream, const Party &party) {
  std::string game_name = "Nothing";
  stream << party.party_id << ", playing " << game_name << ", "
         << party.clients.size() << " clients connected";
  return stream;
}

int generate_party_id() {
  const int random = rand();
  int id = random % 10000;
  while (parties.contains(id)) {
    id++;
  }
  return id;
}
