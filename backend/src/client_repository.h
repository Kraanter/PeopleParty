#include "client.h"

class ClientRepository {
 private:
  std::map<int, Client> clients;

 public:
  ClientRepository();
  Client* CreateClient(std::string name, Party* party);
  void RemoveClient(int client_id);
  bool contains(int client_id);
  bool contains(Client client);
  Client* operator[](int client_id);
};
