#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <uWebSockets/App.h>

#include "socketdata.h"

typedef uWS::WebSocket<true, true, SocketData> WS;

#endif  // TYPEDEFS_H
