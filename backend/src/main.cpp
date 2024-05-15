#include <uWebSockets/App.h>

#include <iostream>

#include "websocket.h"
#include "globals.h"
#include "minigames/crazy_counting/crazy_counting_mini_game.h"

auto main() -> int {
  srand(std::time(0));
  // FIXME: Debug line
  CrazyCountingMiniGame game(10, *parties[0]);
  WebSocket webSocket;
}
