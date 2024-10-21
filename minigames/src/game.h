#ifndef INCLUDE_GPT_GAME_H_
#define INCLUDE_GPT_GAME_H_

#include "debug.h"
#include <google/protobuf/message.h>
#include "gen/game_com.pb.h"
#include <map>
#include <string>

template <typename THostData, typename TPlayerData, typename TGlobalData>
class Game {
public:
  THostData host; // Gebruik van template voor HostData
  std::map<std::string, TPlayerData> players; // Template voor PlayerData
  TGlobalData globalData;                     // Template voor GlobalData

  // Serialiseer het volledige spel
  void Serialize(ProtoGameState *serializedGameState) {
    serializedGameState->set_host(host.Serialize());
    for (const auto &[name, player] : players) {
      std::string playerSerial = player.Serialize();
      D(playerSerial)
      (*serializedGameState->mutable_players())[name] = playerSerial;
    }

    serializedGameState->set_global(globalData.Serialize());
  }

  // Deserialiseer het volledige spel
  void Deserialize(const ProtoGameState &data) {
    // Deel 1: Host Data
    std::string hostData = data.host();
    host.Deserialize(hostData);

    // Deel 2: Player Data
    auto playerData = data.players();

    for (const auto& pair : playerData) {
      TPlayerData player;
      player.Deserialize(pair.second);
      players[pair.first] = player;
    }

    // Deel 3: Global Data
    std::string globalDataStr = data.global();
    globalData.Deserialize(globalDataStr);
  }
};

#endif // INCLUDE_GPT_GAME_H_
