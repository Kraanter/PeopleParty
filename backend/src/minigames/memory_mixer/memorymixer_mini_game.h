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
    Client* host;
    ThreadTimer introduction_timer;
    ThreadTimer results_timer;
    int remaining_time;
    int max_on_card;
    int grid_size;
    int round;
    int max_correct;
    int unique_symbols;
    int active_players;
    MemoryMixer_card target_card;
    int mini_game_phase = 0; // 0 = show all symbols, 1 = show searching symbol and let players guess, 2 = show round results
    std::vector<std::vector<MemoryMixer_card>> grid;
    int memorise_time = 15 SECONDS;
    int guess_time = 10 SECONDS;
    int round_results_time = 5 SECONDS;
private:
    void set_round_difficulty(int current_round, double amount_of_players);
    void create_grid();
    void send_grid(int client_id, bool highlight_correct = false);
    void next_round();
    flatbuffers::Offset<MiniGamePayloadType> build_grid(flatbuffers::FlatBufferBuilder &builder, int phase, bool to_host = true, bool highlight_correct = false);
    void send_round_result(int client_id, std::vector<std::string> wrong_names, std::vector<std::string> correct_names);
    void send_results(int client_id);
    void send_player_submitted(int client_id, bool submitted);
    void process_input(const MiniGamePayloadType* payload, Client* from) override;
};

#endif //PEOPLEPARTY_BACKEND_MEMORYMIXER_MINI_GAME_H