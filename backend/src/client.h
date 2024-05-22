#ifndef CLIENT_H
#define CLIENT_H
#include <uWebSockets/WebSocket.h>

#include "party.h"
#include "typedefs.h"

class Party;

class Client {
 public:
  int client_id;
  std::string name;
  Party *party;
  bool isHost = false;
  bool isSpectating = true;
  WS* ws;

 public:
  Client();
  Client(const std::string name, Party* party);
  Client(const std::string name, Party* party, WS* ws);
  void send(const uint8_t* payload, const size_t size,
            uWS::OpCode opcode = uWS::BINARY) const;
  void send(const std::string& payload, uWS::OpCode opcode = uWS::BINARY) const;
};
bool operator==(const Client& a, const Client& b);
std::ostream& operator<<(std::ostream& stream, const Client& client);
int generate_client_id();

#endif  // CLIENT_H
