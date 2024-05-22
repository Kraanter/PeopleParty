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
  Client *host;
  Game *game;

 public:
  Party();
  void start_game();
  const std::vector<Client*> get_clients();
  const void send_message(const std::function<bool(Client*)>& expression, const std::string& message);
  const void
  send_gamestate(const std::function<bool(Client*)>& expression, flatbuffers::FlatBufferBuilder& builder, flatbuffers::Offset<> gamestate = 0);
  const void send_party_prep(const std::function<bool(Client*)>& expression, flatbuffers::FlatBufferBuilder& builder, flatbuffers::Offset<> partyprep_payload = 0);
  const void send_leaderboard(const std::function<bool(Client*)>& expression, flatbuffers::FlatBufferBuilder& builder, flatbuffers::Offset<> leaderboard_payload = 0);
  const void clients_changed();
};
std::ostream &operator<<(std::ostream &stream, Party &party);
int generate_party_id();

#endif  // PARTY_H
