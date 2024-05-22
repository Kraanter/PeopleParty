#ifndef CLIENT_REPOSITORY_H
#define CLIENT_REPOSITORY_H

#include "client.h"

class ClientRepository {
 private:
  std::map<int, Client> clients;

 public:
  ClientRepository();
  Client* CreateClient(const std::string name, Party* party);
  Client* CreateClient(const std::string name, Party* party, WS* ws);
  void RemoveClient(int client_id);
  bool contains(int client_id);
  bool contains(Client client);
  std::vector<Client*> Find(const std::function<bool(Client*)>& expression);
  Client* operator[](int client_id);
  int size();
};

#endif  // CLIENT_REPOSITORY_H
