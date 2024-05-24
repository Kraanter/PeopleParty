#ifndef GLOBALS_H
#define GLOBALS_H

#include "client_repository.h"
#include "party_repository.h"

inline PartyRepository party_repository;
inline ClientRepository client_repository;
inline uWS::Loop* server_loop;

#endif  // GLOBALS_H
