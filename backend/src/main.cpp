#include <uWebSockets/App.h>

#include <iostream>
#include <sstream>

#include "SocketData.h"
#include "client.h"
#include "room.h"

// different types of serialization / deserialization
#include "flatbuffer/message_generated.h"

std::map<int, Room> rooms;
std::map<int, Client> clients;

void create_room(uWS::WebSocket<true, true, SocketData> *&ws) {
  int room_id = generate_room_id(rooms);
  rooms[room_id] = Room(room_id);
  ws->getUserData()->room = &rooms[room_id];
}

void create_client(const bool host,
                   uWS::WebSocket<true, true, SocketData> *&ws) {
  int client_id = generate_client_id(clients);
  clients[client_id] =
      Client(generate_client_id(clients), ws->getUserData()->room, host, ws);
  ws->getUserData()->client = &clients[client_id];
  rooms[ws->getUserData()->room->room_id].add_client(&clients[client_id]);
}

void remove_client(uWS::WebSocket<true, true, SocketData> *&ws) {
  // Remove client from map
  Client *client = ws->getUserData()->client;
  client->room->remove_client(client);

  if (client->is_host) {
    // Todo: Promote new client to host?
  }

  // Close room if empty
  if (client->room->clients.empty()) rooms.erase(client->room->room_id);

  clients.erase(client->client_id);
}

flatbuffers::Offset<HostPayloadType> buildHostPayload(
    flatbuffers::FlatBufferBuilder &builder, short room_id) {
  return CreateHostPayloadType(builder, room_id);
}

flatbuffers::Offset<JoinPayloadType> buildJoinPayload(
    flatbuffers::FlatBufferBuilder &builder, bool success) {
  return CreateJoinPayloadType(builder, success);
}

void send_host_message(uWS::WebSocket<true, true, SocketData> *&ws) {
  // Populate your game state struct
  flatbuffers::FlatBufferBuilder builder;

  auto payload = buildHostPayload(builder, ws->getUserData()->room->room_id);
  auto message = CreateMessage(builder, MessageType_Host,
                               Payload_HostPayloadType, payload.Union());

  builder.Finish(message);

  uint8_t *buf = builder.GetBufferPointer();

  std::cout << "Sending message of size " << builder.GetSize() << std::endl;

  ws->getUserData()->room->send(buf, builder.GetSize());
}

void send_join_message(uWS::WebSocket<true, true, SocketData> *&ws) {
  // Populate your game state struct
  flatbuffers::FlatBufferBuilder builder;

  auto payload = buildJoinPayload(builder, true);
  auto message = CreateMessage(builder, MessageType_Join,
                               Payload_JoinPayloadType, payload.Union());

  builder.Finish(message);

  uint8_t *buf = builder.GetBufferPointer();

  std::cout << "Sending message of size " << builder.GetSize() << std::endl;

  ws->getUserData()->room->send(buf, builder.GetSize());
}

uint8_t *stringToUint8(const std::string &str) {
  uint8_t *buffer = new uint8_t[str.length()];
  for (size_t i = 0; i < str.length(); ++i) {
    buffer[i] = static_cast<uint8_t>(str[i]);
  }
  return buffer;
}

void process_message(uWS::WebSocket<true, true, SocketData> *&ws,
                     std::string_view message, uWS::OpCode opCode) {
  Room *room = ws->getUserData()->room;
  Client *client = ws->getUserData()->client;

  // Parse the message
  auto parsedMessage = GetMessage(stringToUint8(
      std::string(message)));  // Assuming GetMessage is a generated function

  // Determine the message type
  switch (parsedMessage->type()) {
    case MessageType::MessageType_Host: {
      // Access HostPayload
      auto hostPayload = parsedMessage->payload_as_HostPayloadType();
      // Process host payload...
      break;
    }
    case MessageType::MessageType_Join: {
      // Access JoinPayload
      auto joinPayload = parsedMessage->payload_as_JoinPayloadType();
      // Process join payload...
      break;
    }
    case MessageType::MessageType_GameState: {
      // Access GameStatePayload
      auto gameStatePayload = parsedMessage->payload_as_GameStatePayloadType();
      // Process game state payload...
      break;
    }
  }

  // ws->getUserData()->room->send("message: " + std::string(message));
}

int main() {
  uWS::SSLApp()
      .get("/rooms",
           [](auto *res, auto *req) {
             for (const auto &room : rooms) {
               std::ostringstream ostr;
               ostr << room.second << std::endl;
               res->write(ostr.str());
             }
             res->end();
           })
      .get("/clients",
           [](auto *res, auto *req) {
             for (const auto &client : clients) {
               std::ostringstream ostr;
               ostr << client.second << std::endl;
               res->write(ostr.str());
             }
             res->end();
           })
      .ws<SocketData>(
          "/join/:room",
          {/* Handlers */
           .upgrade =
               [](auto *res, auto *req, auto *context) {
                 int room_id =
                     std::stoi(std::string(req->getParameter("room")));

                 if (!rooms.contains(room_id)) {
                   res->writeStatus("400");
                   res->write("Invalid room code");
                   res->end();
                   return;
                 }

                 res->template upgrade<SocketData>(
                     {.room = &rooms[room_id]},
                     req->getHeader("sec-websocket-key"),
                     req->getHeader("sec-websocket-protocol"),
                     req->getHeader("sec-websocket-extensions"), context);
               },
           .open =
               [](auto *ws) {
                 create_client(false, ws);
                 send_join_message(ws);
               },
           .message =
               [](auto *ws, std::string_view message, uWS::OpCode opCode) {
                 process_message(ws, message, opCode);
               },
           .close = [](auto *ws, int /*code*/,
                       std::string_view /*message*/) { remove_client(ws); }})
      .ws<SocketData>(
          "/host",
          {/* Handlers */
           .open =
               [](auto *ws) {
                 create_room(ws);
                 create_client(true, ws);
                 send_host_message(ws);
               },
           .message =
               [](auto *ws, std::string_view message, uWS::OpCode opCode) {
                 process_message(ws, message, opCode);
               },
           .close = [](auto *ws, int /*code*/,
                       std::string_view /*message*/) { remove_client(ws); }})
      .listen(7899,
              [](auto *listen_socket) {
                if (listen_socket) {
                  std::cout << "Listening on port " << 7899 << std::endl;
                }
              })
      .run();

  std::cout << "Failed to listen on port 7899" << std::endl;
}
