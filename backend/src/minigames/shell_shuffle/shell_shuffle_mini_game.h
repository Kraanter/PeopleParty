// Created by PeopleParty on 2026-03-27.

#ifndef PEOPLEPARTY_BACKEND_SHELL_SHUFFLE_MINI_GAME_H
#define PEOPLEPARTY_BACKEND_SHELL_SHUFFLE_MINI_GAME_H

#include "../minigame.h"
#include "../../flatbuffer/messageClass_generated.h"
#include <map>
#include <vector>
#include <algorithm>
#include <random>

enum class ShellShuffle_Phase : uint8_t
{
    REVEAL = 0,
    SHUFFLE = 1,
    GUESS = 2,
    ROUND_RESULT = 3
};

struct ShellShuffle_Cup
{
    float x_pos;
    float target_x;
    float depth = 0.0f;  // sin-arc depth: +1 = closest (bigger), -1 = farthest (smaller), 0 = idle

    bool is_at_target() const { return std::abs(x_pos - target_x) < 1.0f; }
};

struct ShellShuffle_Player
{
    Client* client = nullptr;
    bool eliminated = false;
    int finished_round = -1;   // -1 = still active; otherwise round they were eliminated
    int guessed_cup = -1;      // -1 = hasn't guessed this round (left-to-right position)
    bool was_correct = false;
};

class ShellShuffle_MiniGame : public MiniGame
{
public:
    ShellShuffle_MiniGame(Game* game);
    ~ShellShuffle_MiniGame() override;

    void start_introduction() override;
    void start_minigame() override;
    void start_result() override;
    void pause() override;
    void resume() override;

    void update(int delta_time) override;
    void process_input(const MiniGamePayloadType* payload, Client* from) override;

    std::string get_display_name() override;
    std::string get_camel_case_name() override;
    std::string get_description() override;

    std::vector<std::pair<Client*, int>> getMinigameResult() override;
    void clients_changed(int client_id, bool joined) override;

private:
    void introduction_update(int dt);
    void update_reveal_phase(int dt);
    void update_shuffle_phase(int dt);
    void update_guess_phase(int dt);
    void update_round_result_phase(int dt);

    void start_reveal_phase();
    void start_shuffle_phase();
    void start_guess_phase();
    void start_round_result_phase();
    void process_round_end();
    void start_next_round();

    void update_cup_positions(int dt);
    void start_next_swap();

    // Returns left-to-right position index (0-based) of the ball cup
    int get_ball_position_index() const;
    // Returns cup array indices sorted by ascending x_pos
    std::vector<int> get_cups_sorted_by_x() const;

    void set_difficulty(int round);
    void reset_cups();
    void reset_player_guesses();
    int count_active_players() const;
    bool all_active_players_guessed() const;

    void send_host_update();
    void send_player_update(int client_id);
    void send_all_player_updates();    // sends to all players regardless of elimination
    void send_round_result_to_host();
    void send_result_data();

private:
    ThreadTimer introduction_timer;
    ThreadTimer result_timer;

    std::map<int, ShellShuffle_Player> players;

    ShellShuffle_Phase current_phase;
    int current_round;
    int remaining_time;

    // Cup state
    std::vector<ShellShuffle_Cup> cups;
    int ball_cup_id;        // array index of the cup that holds the ball (constant per round)

    // Shuffle state
    int swaps_remaining;
    int swap_cup_a;             // -1 if not currently animating a swap
    int swap_cup_b;
    int swap_duration;          // milliseconds each swap takes, regardless of distance
    float current_swap_speed;   // pixels per millisecond for the current swap (distance / swap_duration)
    float swap_total_distance;  // total distance of the current swap (for progress calculation)

    // Per-phase timing
    int reveal_time;
    int guess_time;
    const int round_result_time = 4 SECONDS;

    // Difficulty settings (updated each round)
    int total_swaps;
    int num_cups;

    // Game world constants
    const float MAP_WIDTH = 800.0f;
    const float CUP_MARGIN = 120.0f;

    std::mt19937 rng;
};

#endif //PEOPLEPARTY_BACKEND_SHELL_SHUFFLE_MINI_GAME_H
