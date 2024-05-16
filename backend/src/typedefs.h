#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <uWebSockets/App.h>
#include "socketdata.h"

#define MILLISECONDS * 1
#define SECONDS * 1000
#define MINUTES * 60 SECONDS

typedef uWS::WebSocket<true, true, SocketData> WS;

#endif  // TYPEDEFS_H
