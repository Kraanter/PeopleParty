#include "flatbuffer_message_builder.h"

namespace FlatbufferMessageBuilder {
std::pair<uint8_t *, int> buildHostMessage(int room_id) {
  flatbuffers::FlatBufferBuilder builder;

  auto payload = CreateHostPayloadType(builder, room_id);
  auto message = CreateMessage(builder, MessageType_Host,
                               Payload_HostPayloadType, payload.Union());

  builder.Finish(message);

  uint8_t *buf = builder.GetBufferPointer();

  return {buf, builder.GetSize()};
}

std::pair<uint8_t *, int> buildJoinMessage() {
  flatbuffers::FlatBufferBuilder builder;

  auto payload = CreateJoinPayloadType(builder, true);
  auto message = CreateMessage(builder, MessageType_Join,
                               Payload_JoinPayloadType, payload.Union());

  builder.Finish(message);

  uint8_t *buf = builder.GetBufferPointer();

  return {buf, builder.GetSize()};
}

std::pair<uint8_t *, int> buildGameStateMessage() {
  // TODO: when we have minigames a interface Minigame will be provided as
  // parameter, It will call minigame.buildGameStateMessage() or something than.

  return {nullptr, 0};
}
};  // namespace FlatbufferMessageBuilder
