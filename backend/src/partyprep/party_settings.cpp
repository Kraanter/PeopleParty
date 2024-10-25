#include "party_settings.h"

std::vector<std::string> getMiniGameNames() {
    std::string path = "src/minigames";
    std::vector<std::string> directories;

    // Check if the provided path exists and is a directory
    if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (std::filesystem::is_directory(entry.status())) {
                bool contains_disabled = false;
                  
                  // Check if the directory contains a file named "disabled"
                  for (const auto& file : std::filesystem::directory_iterator(entry.path())) {
                      if (file.path().filename() == "disabled") {
                          contains_disabled = true;
                          break;
                      }
                  }

                  // If "disabled" file is not found, add directory to the list
                  if (!contains_disabled) {
                      directories.push_back(entry.path().filename().string());
                  }
            }
        }
    } else {
        std::cout << "The path does not exist or is not a directory." << std::endl;
    }

    return directories;
}

MiniGameSettings::MiniGameSettings(std::string name) {
    this->name = name;
    this->enabled = true;
}

PartySettings::PartySettings() {
    this->number_of_rounds = 0;
    this->current_round = 0;
    this->game_finished = false;

    // fill playable minigames
    std::vector<std::string> minigame_names = getMiniGameNames();
    for (const auto& name : minigame_names) {
        this->minigames.push_back(new MiniGameSettings(name));
    }
}

void PartySettings::ToggleMiniGame(std::string name, bool enabled) {
    for (auto &minigame : this->minigames) {
        if (minigame->name == name) {
            minigame->enabled = enabled;
            return;
        }
    }
}

void PartySettings::SetNumberOfRounds(int rounds) {
    number_of_rounds = rounds;
}

bool PartySettings::IsMiniGameEnabled(std::string name) {
    for (auto &minigame : this->minigames) {
        if (minigame->name == name) {
            return minigame->enabled;
        }
    }
    return false;
}

std::string PartySettings::GetRandomMiniGame() {
    std::vector<std::string> enabled_minigames;
    for (auto &minigame : this->minigames) {
        if (minigame->enabled) {
            enabled_minigames.push_back(minigame->name);
        }
    }

    if (enabled_minigames.empty()) {
        return "";
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, enabled_minigames.size() - 1);

    return enabled_minigames[dis(gen)];
}
