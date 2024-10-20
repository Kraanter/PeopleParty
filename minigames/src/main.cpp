#include "business_bailout/business_bailout_minigame.h"
#include "debug.h"
#include "gen/game_com.pb.h"
#include <google/protobuf/message.h>
#include <iostream>
#include <ostream>

// Function to read length-prefixed protobuf messages from stdin
bool readLengthPrefixedMessage(Proto_GameUpdate &incoming_update) {
    uint32_t messageLength;

    // Read the length of the incoming message (4 bytes)
    if (!std::cin.read(reinterpret_cast<char*>(&messageLength), sizeof(messageLength))) {
        std::cerr << "Failed to read message length. Input stream might be corrupt." << std::endl;
        return false;
    }

    // Debugging: Output the length we read
    std::cout << "Received message length: " << messageLength << std::endl;

    // Sanity check for message length
    if (messageLength <= 0 || messageLength > 1000000) {  // Arbitrary limit for safety
        std::cerr << "Invalid message length: " << messageLength << std::endl;
        return false;
    }

    // Allocate a buffer to hold the serialized message data
    std::vector<char> buffer(messageLength);

    // Read the actual message data
    if (!std::cin.read(buffer.data(), messageLength)) {
        std::cerr << "Failed to read message data." << std::endl;
        return false;
    }

    // Parse the protobuf message from the buffer
    if (!incoming_update.ParseFromArray(buffer.data(), messageLength)) {
        std::cerr << "Failed to parse protobuf message." << std::endl;
        return false;
    }

    return true;
}
// Function to write serialized protobuf message to stdout
bool writeToStdout(const std::string &outputBuffer) {
    // Write the length of the message first
    uint32_t messageLength = outputBuffer.size();
    if (!std::cout.write(reinterpret_cast<const char*>(&messageLength), sizeof(messageLength))) {
        std::cerr << "Failed to write message length." << std::endl;
        return false;
    }

    // Write the actual message data
    if (!std::cout.write(outputBuffer.data(), messageLength)) {
        std::cerr << "Failed to write message data." << std::endl;
        return false;
    }

    std::cout.flush();  // Ensure the data is flushed to stdout
    return true;
}

auto main() -> int {
  BusinessBailout_Minigame game;

  std::vector<std::string> players;

  players.push_back("kaas");
  players.push_back("stengel");

  auto kaas = game.internalStart(players);

  while (true) {
    Proto_GameUpdate incoming_update;

    std::string inputBuffer;

    // Read and parse the incoming length-prefixed message from stdin
    if (!readLengthPrefixedMessage(incoming_update)) {
      std::cerr << "Failed to read from stdin" << std::endl;
      return 1;
    }

    /*// Parse the binary data into the protobuf message*/
    /*if (!incoming_update.ParseFromString(inputBuffer)) {*/
    /*  std::cout << "failed to parse protobuf message" << std::endl;*/
    /*  return 1;*/
    /*}*/

    // Now you can work with the deserialized message object
    game.update(&incoming_update);

    // Serialize the message to a binary string
    std::string outputBuffer;
    if (!incoming_update.SerializeToString(&outputBuffer)) {
      std::cout << "Failed to serialize protobuf message" << std::endl;
      return 1;
    }

    // Write the serialized message to stdout
    if (!writeToStdout(outputBuffer)) {
      std::cout << "Failed to write message to stdout" << std::endl;
      return 1;
    }
  }
  return 0;
}
