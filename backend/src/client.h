#ifndef CLIENT_H
#define CLIENT_H
#include <uWebSockets/WebSocket.h>

#include "party.h"
#include "role.h"
#include "typedefs.h"

class Client {
 public:
  int client_id;
  std::string name;
  const Party* party;
  Role role;
  WS* ws;

 public:
  Client(const std::string name, const Party* party);
  void send(const uint8_t* payload, const size_t size,
            uWS::OpCode opcode = uWS::BINARY) const;
  void send(const std::string& payload, uWS::OpCode opcode = uWS::BINARY) const;
};
bool operator==(const Client& a, const Client& b);
std::ostream& operator<<(std::ostream& stream, const Client& client);
int generate_client_id();

#endif  // CLIENT_H
