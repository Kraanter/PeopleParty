#ifndef SOCKETDATA_H
#define SOCKETDATA_H

#include "client.h"

struct SocketData {
  Client* client;
  int party_id;
};

#endif  // SOCKETDATA_H
