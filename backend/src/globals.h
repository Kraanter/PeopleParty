#ifndef GLOBALS_H
#define GLOBALS_H

#include "client_repository.h"
#include "party_repository.h"

inline PartyRepository parties;
inline ClientRepository clients;
inline uWS::Loop* server_loop;

#endif  // GLOBALS_H
