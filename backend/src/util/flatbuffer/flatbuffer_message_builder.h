#ifndef FLATBUFFER_MESSAGE_BUILDER_H
#define FLATBUFFER_MESSAGE_BUILDER_H

#include "../../flatbuffer/messageClass_generated.h"

namespace FlatbufferMessageBuilder {
std::string buildHostMessage(int room_id);
std::string buildJoinMessage();
};

#endif  // FLATBUFFER_MESSAGE_BUILDER_H
