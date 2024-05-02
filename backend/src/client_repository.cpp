#include "client_repository.h"

ClientRepository::ClientRepository() {}

Client* ClientRepository::CreateClient(const std::string name,
                                       const Party* party) {
  Client client = Client(name, party);
  clients[client.client_id] = client;
  return &clients[client.client_id];
}

void ClientRepository::RemoveClient(int client_id) { clients.erase(client_id); }

bool ClientRepository::contains(int client_id) {
  return clients.contains(client_id);
}

bool ClientRepository::contains(Client client) {
  return clients.contains(client.client_id);
}

Client* ClientRepository::operator[](int client_id) {
  return &clients[client_id];
}
