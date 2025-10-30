#ifndef SOCKETDATA_H
#define SOCKETDATA_H

#include "../client.h"

class Client;

struct SocketData {
  Client* client;
  int party_id;
  bool isLocalhost = false;
};

#endif  // SOCKETDATA_H
