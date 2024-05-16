#ifndef PARTY_H
#define PARTY_H
#include <string>
#include <vector>

#include "game.h"
#include "client.h"

class Game;
class Client;

class Party {
 public:
  int party_id;
  std::vector<Client *> clients;
  Client *host;
  Game *game;

 public:
  Party();
  void add_client(Client *client);
  void remove_client(Client *client);
  void promote_client(Client *client);
  void send(const uint8_t *payload, size_t size,
            uWS::OpCode opcode = uWS::BINARY);
  void send(const std::string, uWS::OpCode opcode = uWS::BINARY);
};
std::ostream &operator<<(std::ostream &stream, const Party &party);
int generate_party_id();

#endif  // PARTY_H
