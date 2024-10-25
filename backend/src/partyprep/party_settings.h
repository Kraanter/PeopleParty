#ifndef Party_Settings_H
#define Party_Settings_H

#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <random>

class MiniGameSettings {
public:
    MiniGameSettings(std::string name);
public:
    std::string name;
    bool enabled;
};


class PartySettings {
public:
    PartySettings();
    void ToggleMiniGame(std::string name, bool enabled);
    void SetNumberOfRounds(int rounds);
    bool IsMiniGameEnabled(std::string name);
    std::string GetRandomMiniGame();
public:
    int number_of_rounds; // 0 = infinite
    int current_round;
    std::vector<MiniGameSettings*> minigames; // minigames that will be played
};

#endif  // Party_Settings_H