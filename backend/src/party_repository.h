#ifndef PARTY_REPOSITORY_H
#define PARTY_REPOSITORY_H

#include "party.h"

class PartyRepository {
 private:
  std::map<int, Party> parties;

 public:
  PartyRepository();
  Party* CreateParty();
  void RemoveParty(int party_id);
  bool contains(int party_id);
  bool contains(Party party);
  Party* operator[](int party);
};

#endif  // PARTY_REPOSITORY_H
