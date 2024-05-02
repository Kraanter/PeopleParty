#include "websocket.h"

#include "client.h"
#include "flatbuffer_message_builder.h"
#include "globals.h"
#include "socketdata.h"

WebSocket::WebSocket() { this->init(); }

void send_host_message(WS *ws) {
  std::pair<uint8_t *, int> output =
      FlatbufferMessageBuilder::buildHostMessage(ws->getUserData()->party_id);
  Party *p = parties[ws->getUserData()->party_id];

  p->send(output.first, output.second);
}

void send_join_message(WS *ws) {
  std::pair<uint8_t *, int> output =
      FlatbufferMessageBuilder::buildJoinMessage();

  ws->getUserData()->client->send(output.first, output.second);
}

void WebSocket::init() {
  uWS::SSLApp()
      .ws<SocketData>(
          "/join/:room/:name",
          {/* Handlers */
           .upgrade =
               [](auto *res, auto *req, auto *context) {
                 int party_id =
                     std::stoi(std::string(req->getParameter("room")));
                 std::string client_name =
                     std::string(req->getParameter("name"));

                 if (!parties.contains(party_id)) {
                   res->writeStatus("400");
                   res->write("Invalid room code");
                   res->end();
                   return;
                 };

                 Client *c =
                     clients.CreateClient(client_name, parties[party_id]);

                 res->template upgrade<SocketData>(
                     {.client = c, .party_id = party_id},
                     req->getHeader("sec-websocket-key"),
                     req->getHeader("sec-websocket-protocol"),
                     req->getHeader("sec-websocket-extensions"), context);
               },
           .open =
               [](auto *ws) {
                 ws->getUserData()->client->ws = ws;
                 send_join_message(ws);
               },
           .message =
               [](auto *ws, std::string_view message, uWS::OpCode opCode) {
                 //  process_message(ws, message, opCode);
               },
           .close =
               [](auto *ws, int /*code*/, std::string_view /*message*/) {
                 clients.RemoveClient(ws->getUserData()->client->client_id);
               }})
      .ws<SocketData>(
          "/host",
          {/* Handlers */
           .open =
               [](auto *ws) {
                 Party *p = parties.CreateParty();
                 Client *c = clients.CreateClient("HOST", p);
                 p->host = c;
                 send_host_message(ws);
               },
           .message =
               [](auto *ws, std::string_view message, uWS::OpCode opCode) {
                 // process_message(ws, message, opCode);
               },
           .close =
               [](auto *ws, int /*code*/, std::string_view /*message*/) {
                 parties.RemoveParty(ws->getUserData()->party_id);
               }})
      .listen(7899,
              [](auto *listen_socket) {
                if (listen_socket) {
                  std::cout << "Listening on port " << 7899 << std::endl;
                }
              })
      .run();
}
