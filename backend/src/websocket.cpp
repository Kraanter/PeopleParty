#include "websocket.h"

#include "client.h"
#include "flatbuffer_message_builder.h"
#include "globals.h"
#include "socketdata.h"

WebSocket::WebSocket() { this->init(); }

void send_host_message(WS *ws) {
  std::string output = FlatbufferMessageBuilder::buildHostMessage(ws->getUserData()->party_id);
  
  ws->getUserData()->client->send(output);
}

void send_join_message(WS *ws) {
  std::string output = FlatbufferMessageBuilder::buildJoinMessage();

  ws->getUserData()->client->send(output);
}

void WebSocket::init() {
  uWS::SSLApp()
      // Catch all route
      .any("/*", [](auto *res, auto *req) {
        std::cout << "catch all route" << std::endl;
        res->writeStatus("404");
        res->write("Not Found");
        res->end();
      })
      .ws<SocketData>(
          "/join/:room/:name",
          {/* Handlers */
           .upgrade =
               [](auto *res, auto *req, auto *context) {
                std::cout << "connection started with join" << std::endl;

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

                 Client *c = clients.CreateClient(client_name, parties[party_id]);

                 parties[party_id]->add_client(c);

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
                 std::cout << "connection started with host" << std::endl;
                 Party *p = parties.CreateParty();
                 Client *c = clients.CreateClient("HOST", p, ws);
                 p->host = c;

                 ws->getUserData()->client = c;
                 ws->getUserData()->party_id = p->party_id;

                 send_host_message(ws);
               },
           .message =
               [](auto *ws, std::string_view message, uWS::OpCode opCode) {
                 // process_message(ws, message, opCode);
               },
           .close =
               [](auto *ws, int /*code*/, std::string_view /*message*/) {
                // wait for 5 seconds before removing the party
                std::thread([ws]() {
                  std::this_thread::sleep_for(std::chrono::seconds(5));
                    parties.RemoveParty(ws->getUserData()->party_id);
                }).detach();
               }})
      .listen(7899,
              [](auto *listen_socket) {
                if (listen_socket) {
                  std::cout << "Listening on port " << 7899 << std::endl;
                }
              })
      .run();
}
