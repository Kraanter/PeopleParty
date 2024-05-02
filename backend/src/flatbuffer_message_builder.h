#ifndef FLATBUFFER_MESSAGE_BUILDER_H
#define FLATBUFFER_MESSAGE_BUILDER_H

#include "flatbuffer/messageClass_generated.h"

namespace FlatbufferMessageBuilder {
std::pair<uint8_t*, int> buildHostMessage(int room_id);
std::pair<uint8_t*, int> buildJoinMessage();

// TODO: when we have minigames a interface Minigame will be provided as
// parameter, in the cpp it will call the minigame's buildGameStateMessage
std::pair<uint8_t*, int> buildGameStateMessage();
};  // namespace FlatbufferMessageBuilder

#endif  // FLATBUFFER_MESSAGE_BUILDER_H
