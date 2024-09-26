#ifndef INCLUDE_GPT_GAME_H_
#define INCLUDE_GPT_GAME_H_

#include <map>
#include <string>
#include <sstream>

template <typename THostData, typename TPlayerData, typename TGlobalData>
class Game {
public:
  THostData host; // Gebruik van template voor HostData
  std::map<std::string, TPlayerData> players; // Template voor PlayerData
  TGlobalData globalData;                     // Template voor GlobalData

  // Serialiseer het volledige spel
  std::string Serialize() {
    std::string result = host.Serialize() + "|\n";
    for (const auto &[name, player] : players) {
      result += name + ":" + player.Serialize() + "\n";
    }
    result += "|\n" + globalData.Serialize() + "|\n";
    return result;
  }

  // Deserialiseer het volledige spel
  void Deserialize(const std::string &data) {
    size_t hostEnd = data.find("|\n");
    size_t playersEnd =
        data.find("|\n", hostEnd + 2); // Volgende scheiding voor players
    size_t globalStart = data.rfind("|\n");

    if (hostEnd == std::string::npos || playersEnd == std::string::npos ||
        globalStart == std::string::npos || hostEnd >= playersEnd ||
        playersEnd >= globalStart) {
      throw std::runtime_error("Invalid serialized format");
    }

    // Deel 1: Host Data
    std::string hostData = data.substr(0, hostEnd);
    host.Deserialize(hostData);

    // Deel 2: Player Data
    std::string playerData = data.substr(
        hostEnd + 2, playersEnd - (hostEnd + 2)); // Pak de spelerdata
    std::istringstream playerStream(playerData);
    std::string line;
    while (std::getline(playerStream, line)) {
      size_t separator = line.find(':');
      if (separator == std::string::npos) {
        throw std::runtime_error("Invalid player format");
      }

      std::string playerName = line.substr(0, separator);
      std::string playerSerializedData = line.substr(separator + 1);

      // Deserialiseer de speler
      TPlayerData player;
      player.Deserialize(playerSerializedData);
      players[playerName] = player; // Voeg de speler toe aan de map
    }

    // Deel 3: Global Data
    std::string globalDataStr = data.substr(
        playersEnd + 2,
        globalStart - (playersEnd +
                       2)); // Globale data tussen tweede en laatste scheiding
    globalData.Deserialize(globalDataStr);
  }
};

#endif // INCLUDE_GPT_GAME_H_
