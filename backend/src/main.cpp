#include <uWebSockets/App.h>

#include <iostream>

#include "websocket.h"
#include "globals.h"
#include "minigames/crazy_counting/game.h"

auto main() -> int {
  srand(std::time(0));
  // FIXME: Debug line
  CrazyCountingGame game(10, *parties[0]);
  WebSocket webSocket;
}
