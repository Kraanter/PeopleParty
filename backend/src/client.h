#ifndef CLIENT_H
#define CLIENT_H
#include <uWebSockets/WebSocket.h>

#include "SocketData.h"

class Client {
 public:
  int client_id;
  std::string name;
  Room* room;
  bool is_host;
  Client();
  Client(int client, Room* room, bool host,
         uWS::WebSocket<true, true, SocketData>* ws);
  void send(const uint8_t* payload, size_t size,
            uWS::OpCode opcode = uWS::TEXT) const;

 private:
  uWS::WebSocket<true, true, SocketData>* ws;
};
bool operator==(const Client& a, const Client& b);
std::ostream& operator<<(std::ostream& stream, const Client& client);

int generate_client_id(const std::map<int, Client>& clients);

#endif  // CLIENT_H
