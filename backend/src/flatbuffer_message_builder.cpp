#include "flatbuffer_message_builder.h"
#include <iostream>

namespace FlatbufferMessageBuilder {
std::string buildHostMessage(int room_id) {
  flatbuffers::FlatBufferBuilder builder;

  auto payload = CreateHostPayloadType(builder, room_id);
  auto message = CreateMessage(builder, MessageType_Host,
                               Payload_HostPayloadType, payload.Union());

  builder.Finish(message);

  uint8_t *buf = builder.GetBufferPointer();

  std::string dataAsString = std::string(reinterpret_cast<const char*>(buf), builder.GetSize());

  return dataAsString;
}

std::string buildJoinMessage() {
  flatbuffers::FlatBufferBuilder builder;

  auto payload = CreateJoinPayloadType(builder, true);
  auto message = CreateMessage(builder, MessageType_Join,
                               Payload_JoinPayloadType, payload.Union());

  builder.Finish(message);

  uint8_t *buf = builder.GetBufferPointer();

  std::string dataAsString = std::string(reinterpret_cast<const char*>(buf), builder.GetSize());

  return dataAsString;
}

std::string buildGameStateMessage() {
  // TODO: when we have minigames a interface Minigame will be provided as
  // parameter, It will call minigame.buildGameStateMessage() or something than.

  return "";
}
};  // namespace FlatbufferMessageBuilder
