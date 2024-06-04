#ifndef PEOPLEPARTY_BACKEND_MEMORYMIXER_MINI_GAME_H
#define PEOPLEPARTY_BACKEND_MEMORYMIXER_MINI_GAME_H

#include "../../flatbuffer/messageClass_generated.h"
#include "../minigame.h"
#include "memorymixer_player.h"
#include "memorymixer_card.h"
#include <map>

class MemoryMixer_MiniGame : public MiniGame {
public:
    MemoryMixer_MiniGame(Game* game);
    ~MemoryMixer_MiniGame();
    void start_introduction() override;
    void start_minigame() override;
    void start_result() override;
    void update(int delta_time) override;
    void introduction_update(int delta_time);
    std::vector<Client*> getMinigameResult() override;
    void clients_changed(int client_id, bool joined) override;
    std::string get_display_name() override;
    std::string get_camel_case_name() override;
    std::string get_description() override;
private:
    std::map<int, MemoryMixer_Player> players;
    ThreadTimer introduction_timer;
    ThreadTimer results_timer;
    int remaining_time;
    int max_on_card;
    int grid_size;
    int mini_game_phase = 0; // 0 = show all symbols, 1 = show searching symbol, 2 = show hidden grid
    std::vector<std::vector<MemoryMixer_card>> grid;
private:
    void send_grid();
    void send_player_submitted(int client_id);
    void process_input(const MiniGamePayloadType* payload, Client* from) override;
};

#endif //PEOPLEPARTY_BACKEND_MEMORYMIXER_MINI_GAME_H