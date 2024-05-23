#include "client_repository.h"

ClientRepository::ClientRepository() {}

Client* ClientRepository::CreateClient(const std::string name,
                                       Party* party) {
    return CreateClient(name, party, nullptr);
}

Client* ClientRepository::CreateClient(const std::string name,
                                       Party* party, WS* ws) {
  Client client = Client(name, party, ws);
  clients[client.client_id] = client;
  party->clients_changed(client.client_id, true);
  return &clients[client.client_id];
}

void ClientRepository::RemoveClient(int client_id) {
    Party* p = clients[client_id].party;
    if (p != nullptr && p->get_clients().size() > 1) {
        clients.erase(client_id);
        p->clients_changed(client_id, false);
        return;
    }
    clients.erase(client_id);
}

bool ClientRepository::contains(int client_id) {
  return clients.contains(client_id);
}

bool ClientRepository::contains(Client client) {
  return clients.contains(client.client_id);
}

std::vector<Client*> ClientRepository::Find(const std::function<bool(Client*)>& expression) {
  std::vector<Client*> result;
  for (auto& [_, client] : clients) {
    if (expression(&client)) {
      result.push_back(&client);
    }
  }
  return result;
}

Client* ClientRepository::operator[](int client_id) {
  return &clients[client_id];
}

int ClientRepository::size() {
    return clients.size();
}
