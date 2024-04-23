#include "client.h"

#include "room.h"

Client::Client() {
  client_id = -1;
  name = "-1";
  room = nullptr;
  is_host = false;
  ws = nullptr;
}

Client::Client(int client, Room* room, bool host,
               uWS::WebSocket<true, true, SocketData>* ws) {
  client_id = client;
  name = std::to_string(client_id);
  this->room = room;
  is_host = host;
  this->ws = ws;
}

void Client::send(const uint8_t* payload, size_t size,
                  uWS::OpCode opcode) const {
  // convert the received uint8_t to a char pointer to a string
  std::string dataAsString =
      std::string(reinterpret_cast<const char*>(payload), size);

  ws->send(dataAsString, opcode);
}

bool operator==(const Client& a, const Client& b) {
  return a.client_id == b.client_id;
}

std::ostream& operator<<(std::ostream& stream, const Client& client) {
  stream << client.name << "(i" << client.client_id << "r"
         << client.room->room_id << ")";
  return stream;
}

int generate_client_id(const std::map<int, Client>& clients) {
  int id = 0;
  while (clients.contains(id)) id++;
  return id;
}
