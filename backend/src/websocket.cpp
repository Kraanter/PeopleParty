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

void process_message(WS *ws, std::string_view message) {
  const uint8_t *buffer = reinterpret_cast<const uint8_t *>(message.data());
  auto parsedMessage = GetMessage(buffer);

  // if (!VerifyMessageBuffer(flatbuffers::Verifier(buffer, message.size()))) {
  //   std::cout << "Message verification failed" << std::endl;
  //   return;
  // }

  Game* targetGame = party_repository[ws->getUserData()->party_id]->game;
  if (targetGame == nullptr) {
    return;
  }

  targetGame->process_input(parsedMessage, ws->getUserData()->client);
}

bool is_numeric(const std::string &s) {
  return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
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
               [](auto *res, uWS::HttpRequest *req, auto *context) {
                std::cout << "connection started with join" << std::endl;

                std::string room = std::string(req->getParameter("room"));
                std::string name = std::string(req->getParameter("name"));

                if (room == "" || !is_numeric(room) || name == "" ) {
                  res->writeStatus("400");
                  res->write("Invalid parameters");
                  res->end();
                  return;
                }

                 int party_id = std::stoi(room);

                 bool uniuqeName = true;
                  for (auto &client : party_repository[party_id]->get_clients()) {
                    if (client->name == name) {
                      uniuqeName = false;
                      break;
                    }
                  }
                 
                 if (!party_repository.contains(party_id) || !uniuqeName) {
                   res->writeStatus("400");
                   res->write("Invalid room code");
                   res->end();
                   return;
                 };

                 Client *c = client_repository.CreateClient(name, party_repository[party_id]);

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
                 process_message(ws, message);
               },
           .close =
               [](auto *ws, int /*code*/, std::string_view /*message*/) {
                 ws->getUserData()->client->ws == nullptr;
                 client_repository.RemoveClient(ws->getUserData()->client->client_id);
               }})
      .ws<SocketData>(
          "/host",
          {/* Handlers */
           .open =
               [](auto *ws) {
                 std::cout << "connection started with host" << std::endl;
                 Party *p = party_repository.CreateParty();
                 Client *c = client_repository.CreateClient("HOST", p, ws);
                 c->isHost = true;
                 p->host = c;

                 ws->getUserData()->client = c;
                 ws->getUserData()->party_id = p->party_id;

                 send_host_message(ws);
                 p->start_game();
               },
           .message =
               [](auto *ws, std::string_view message, uWS::OpCode opCode) {
                 process_message(ws, message);
               },
           .close =
               [](auto *ws, int /*code*/, std::string_view /*message*/) {
                ws->getUserData()->client->party->stop_game();
                party_repository.RemoveParty(ws->getUserData()->party_id);
               }})
      .listen(7899,
              [](auto *listen_socket) {
                server_loop = uWS::Loop::get();
                if (listen_socket) {
                  std::cout << "Listening on port " << 7899 << std::endl;
                }
              })
      .run();
}
