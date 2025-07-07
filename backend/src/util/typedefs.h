#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <uWebSockets/App.h>
#include "socketdata.h"

using WS = uWS::WebSocket<true, true, SocketData>;

#endif  // TYPEDEFS_H
