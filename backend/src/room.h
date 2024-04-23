#ifndef ROOM_H
#define ROOM_H
#include <map>
#include <string>
#include <vector>

#include "client.h"

class Client;

class Room {
 public:
  int room_id;
  std::vector<Client*> clients;
  Room();
  Room(int id);
  void add_client(Client* client);
  void remove_client(Client* client);
  void send(const uint8_t* payload, size_t size,
            uWS::OpCode opcode = uWS::TEXT);
};
std::ostream& operator<<(std::ostream& stream, const Room& room);
int generate_room_id(const std::map<int, Room>& rooms);

#endif  // ROOM_H
