#ifndef SOCKETDATA_H
#define SOCKETDATA_H

// Forward declaration because otherwise we get some funky cirular linking
class Client;
class Room;

struct SocketData {
  Client* client;
  Room* room;
};

#endif  // SOCKETDATA_H
