#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <uWebSockets/App.h>
#include "thread_timer.h"

class WebSocket {
public:
  WebSocket();
  void init();
private:
  ThreadTimer timer;
};

#endif  // WEBSOCKET_H
