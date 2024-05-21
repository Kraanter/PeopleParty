#include "client.h"

#include "globals.h"
#include "party.h"

Client::Client() : Client("", nullptr) {
}

Client::Client(const std::string name, const Party* party) : Client(name, party, nullptr) {
}

Client::Client(const std::string name, const Party* party, WS* ws) {
  this->client_id = generate_client_id();
  this->name = name;
  this->party = party;
  this->ws = ws;
}

void Client::send(const uint8_t* payload, const size_t size,
                  uWS::OpCode opcode) const {
  // convert the received uint8_t to a char pointer to a string
  std::string dataAsString =
      std::string(reinterpret_cast<const char*>(payload), size);

  ws->send(dataAsString, opcode);
}

void Client::send(const std::string& payload, uWS::OpCode opcode) const {
  ws->send(payload, opcode);
}

bool operator==(const Client& a, const Client& b) { return &a == &b; }

std::ostream& operator<<(std::ostream& stream, const Client& client) {
  stream << client.name << " (" << client.party->party_id << ")";
  return stream;
}

int generate_client_id() {
  int id = 0;
  while (client_repository.contains(id)) id++;
  return id;
}
