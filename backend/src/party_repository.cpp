#include "party_repository.h"

#include "globals.h"

PartyRepository::PartyRepository() {}

Party* PartyRepository::CreateParty() {
  Party* p = new Party();
  parties[p->party_id] = *p;
  return &parties[p->party_id];
}

void PartyRepository::RemoveParty(int party_id) {
  // remove every client in the party
  Party* p = &parties[party_id];

  std::vector<Client*> clients = p->get_clients();
  for (int i = 0; i < clients.size(); i++) {
    client_repository.RemoveClient(clients[i]->client_id);
  }

  parties.erase(party_id);
}

bool PartyRepository::contains(Party party) {
  return parties.contains(party.party_id);
}

bool PartyRepository::contains(int party_id) {
  return parties.contains(party_id);
}

Party* PartyRepository::operator[](int party_id) { return &parties[party_id]; }

std::vector<Party*> PartyRepository::GetParties() {
  std::vector<Party*> party_list;
  for (auto& party : parties) {
    party_list.push_back(&party.second);
  }
  return party_list;
}
