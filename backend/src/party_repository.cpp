#include "party_repository.h"

#include "globals.h"

PartyRepository::PartyRepository() {}

Party* PartyRepository::CreateParty() {
  Party p;
  parties[p.party_id] = p;
  return &parties[p.party_id];
}

void PartyRepository::RemoveParty(int party_id) {
  // remove every client in the party
  parties.erase(party_id);
}

bool PartyRepository::contains(Party party) {
  return parties.contains(party.party_id);
}

bool PartyRepository::contains(int party_id) {
  return parties.contains(party_id);
}

Party* PartyRepository::operator[](int party_id) { return &parties[party_id]; }
