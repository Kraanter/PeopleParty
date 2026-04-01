// Created by PeopleParty on 2026-03-27.

#include "shell_game_mini_game.h"
#include "../../game.h"
#include "../../util/globals.h"
#include <cmath>
#include <climits>

ShellGame_MiniGame::ShellGame_MiniGame(Game* game) : MiniGame(game)
{
    min_players = 2;
    max_players = -1;

    current_phase = ShellGame_Phase::REVEAL;
    current_round = 0;
    remaining_time = 0;
    ball_cup_id = 0;
    swaps_remaining = 0;
    swap_cup_a = -1;
    swap_cup_b = -1;
    swap_duration = 700;
    current_swap_speed = 0.0f;
    reveal_time = 4 SECONDS;
    guess_time = 8 SECONDS;
    total_swaps = 5;
    num_cups = 3;

    rng = std::mt19937(std::random_device{}());
}

ShellGame_MiniGame::~ShellGame_MiniGame()
{
    introduction_timer.clear();
    timer.clear();
    result_timer.clear();
}

// ─── Introduction ────────────────────────────────────────────────────────────

void ShellGame_MiniGame::start_introduction()
{
    update_interval = 500 MILLISECONDS;
    introduction_timer.setInterval([this]()
    {
        introduction_update(update_interval);
    }, update_interval);
}

void ShellGame_MiniGame::introduction_update(int dt)
{
    introduction_time -= dt;
    send_minigame_introduction(
        get_camel_case_name(),
        introduction_time,
        get_display_name(),
        get_description()
    );
    if (introduction_time <= 0)
    {
        introduction_timer.clear();
        start_minigame();
    }
}

// ─── Game start ──────────────────────────────────────────────────────────────

void ShellGame_MiniGame::start_minigame()
{
    for (auto* client : game->get_clients())
    {
        if (client->party->host == client) continue;
        players[client->client_id] = ShellGame_Player{};
        players[client->client_id].client = client;
    }

    current_round = 1;
    set_difficulty(current_round);
    reset_cups();
    start_reveal_phase();

    update_interval = 50 MILLISECONDS;
    timer.setInterval([this]()
    {
        update(update_interval);
    }, update_interval);
}

// ─── Main update loop ────────────────────────────────────────────────────────

void ShellGame_MiniGame::update(int dt)
{
    switch (current_phase)
    {
        case ShellGame_Phase::REVEAL:
            update_reveal_phase(dt);
            break;
        case ShellGame_Phase::SHUFFLE:
            update_shuffle_phase(dt);
            break;
        case ShellGame_Phase::GUESS:
            update_guess_phase(dt);
            break;
        case ShellGame_Phase::ROUND_RESULT:
            update_round_result_phase(dt);
            return;
            break;
    }

    send_host_update();
    send_all_player_updates();
}

void ShellGame_MiniGame::update_reveal_phase(int dt)
{
    remaining_time -= dt;
    if (remaining_time <= 0)
    {
        start_shuffle_phase();
    }
}

void ShellGame_MiniGame::update_shuffle_phase(int dt)
{
    update_cup_positions(dt);

    if (swap_cup_a == -1)
    {
        if (swaps_remaining > 0)
        {
            start_next_swap();
        }
        else
        {
            start_guess_phase();
        }
    }
}

void ShellGame_MiniGame::update_guess_phase(int dt)
{
    remaining_time -= dt;
    if (remaining_time <= 0 || all_active_players_guessed())
    {
        start_round_result_phase();
    }
}

void ShellGame_MiniGame::update_round_result_phase(int dt)
{
    remaining_time -= dt;
    send_all_player_updates();
    if (remaining_time <= 0)
    {
        process_round_end();
    }
}

// ─── Phase transitions ───────────────────────────────────────────────────────

void ShellGame_MiniGame::start_reveal_phase()
{
    current_phase = ShellGame_Phase::REVEAL;
    remaining_time = reveal_time;
    send_all_player_updates();
}

void ShellGame_MiniGame::start_shuffle_phase()
{
    current_phase = ShellGame_Phase::SHUFFLE;
    remaining_time = -1;
    swaps_remaining = total_swaps;
    swap_cup_a = -1;
    swap_cup_b = -1;
    send_all_player_updates();
    start_next_swap();
}

void ShellGame_MiniGame::start_guess_phase()
{
    current_phase = ShellGame_Phase::GUESS;
    remaining_time = guess_time;
    send_all_player_updates();
}

void ShellGame_MiniGame::start_round_result_phase()
{
    current_phase = ShellGame_Phase::ROUND_RESULT;
    remaining_time = round_result_time;

    // Determine correct position and evaluate guesses
    int ball_pos = get_ball_position_index();
    for (auto& [id, player] : players)
    {
        if (player.eliminated) continue;
        player.was_correct = (player.guessed_cup == ball_pos);
        if (!player.was_correct)
        {
            player.eliminated = true;
            player.finished_round = current_round;
        }
    }

    send_round_result_to_host();
    send_all_player_updates();
}

void ShellGame_MiniGame::process_round_end()
{
    if (count_active_players() <= 1)
    {
        start_result();
    }
    else
    {
        start_next_round();
    }
}

void ShellGame_MiniGame::start_next_round()
{
    current_round++;
    set_difficulty(current_round);
    reset_cups();
    reset_player_guesses();
    start_reveal_phase();
}

// ─── Result ──────────────────────────────────────────────────────────────────

void ShellGame_MiniGame::start_result()
{
    timer.clear();
    send_result_data();
    result_timer.setTimeout([this]()
    {
        finished();
    }, 6 SECONDS);
}

// ─── Cup animation ───────────────────────────────────────────────────────────

void ShellGame_MiniGame::update_cup_positions(int dt)
{
    if (swap_cup_a == -1) return;

    float delta = current_swap_speed * dt;

    for (int idx : {swap_cup_a, swap_cup_b})
    {
        float diff = cups[idx].target_x - cups[idx].x_pos;
        if (std::abs(diff) <= delta)
        {
            cups[idx].x_pos = cups[idx].target_x;
        }
        else
        {
            cups[idx].x_pos += (diff > 0 ? 1.0f : -1.0f) * delta;
        }
    }

    // Compute depth arc: sin(progress * π), peaks at midpoint of the swap
    if (swap_total_distance > 0.0f)
    {
        float remaining = std::abs(cups[swap_cup_a].target_x - cups[swap_cup_a].x_pos);
        float progress = 1.0f - (remaining / swap_total_distance);
        float arc = std::sin(progress * M_PI);
        cups[swap_cup_a].depth =  arc;   // closer cup: grows
        cups[swap_cup_b].depth = -arc;   // farther cup: shrinks
    }

    if (cups[swap_cup_a].is_at_target() && cups[swap_cup_b].is_at_target())
    {
        cups[swap_cup_a].depth = 0.0f;
        cups[swap_cup_b].depth = 0.0f;
        swaps_remaining--;
        swap_cup_a = -1;
        swap_cup_b = -1;
    }
}

void ShellGame_MiniGame::start_next_swap()
{
    if (cups.size() < 2) return;

    int a, b;
    std::uniform_int_distribution<int> dist(0, (int)cups.size() - 1);

    // Avoid repeating the same pair as last swap if possible
    int attempts = 0;
    do
    {
        a = dist(rng);
        b = dist(rng);
        attempts++;
    } while (a == b && attempts < 10);

    if (a == b) return;

    swap_cup_a = a;
    swap_cup_b = b;

    // Set targets to each other's current positions
    float target_a = cups[b].x_pos;
    float target_b = cups[a].x_pos;
    cups[a].target_x = target_a;
    cups[b].target_x = target_b;

    // Record total swap distance, compute per-swap speed so duration is constant
    swap_total_distance = std::abs(target_a - cups[a].x_pos);
    current_swap_speed = (swap_duration > 0 && swap_total_distance > 0.0f)
        ? swap_total_distance / static_cast<float>(swap_duration)
        : 1.0f;
    cups[a].depth = 0.0f;
    cups[b].depth = 0.0f;
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

int ShellGame_MiniGame::get_ball_position_index() const
{
    auto sorted = get_cups_sorted_by_x();
    for (int i = 0; i < (int)sorted.size(); i++)
    {
        if (sorted[i] == ball_cup_id) return i;
    }
    return 0;
}

std::vector<int> ShellGame_MiniGame::get_cups_sorted_by_x() const
{
    std::vector<int> indices((int)cups.size());
    for (int i = 0; i < (int)cups.size(); i++) indices[i] = i;
    std::sort(indices.begin(), indices.end(), [this](int a, int b)
    {
        return cups[a].x_pos < cups[b].x_pos;
    });
    return indices;
}

void ShellGame_MiniGame::set_difficulty(int round)
{
    // Number of cups: starts at 3, +1 every 3 rounds, max 6
    num_cups = std::min(3 + (round - 1) / 3, 6);

    // Swap duration (ms): each swap takes this long regardless of cup distance
    // on the rounds when a cup is added, swaps are a bit slower to help players adjust
    if (round % 3 == 1 && round != 1)
    {
        // add some swap time during the rounds a cup is added
        swap_duration = std::min(swap_duration + 50, 1000);
    } else if (round < 8) {
        // slightly decrease swap time
        swap_duration = std::max(250, swap_duration - 50);
    } else {
        // even less decrease
        swap_duration = std::max(250, swap_duration - 25);
    }

    // Number of swaps: 5 base + 1 per round, capped at 22
    total_swaps = std::min(4 + round, 22);

    // Reveal time: 4s early, 3s later
    reveal_time = (round <= 4) ? 4 SECONDS : 3 SECONDS;

    // Guess time: starts at 8s, decreases by 200ms per round, min 4s
    guess_time = std::max(4000, 8000 - (round - 1) * 200);
}

void ShellGame_MiniGame::reset_cups()
{
    cups.clear();

    float usable_width = MAP_WIDTH - 2.0f * CUP_MARGIN;
    float spacing = (num_cups > 1) ? usable_width / (float)(num_cups - 1) : 0.0f;

    for (int i = 0; i < num_cups; i++)
    {
        ShellGame_Cup cup;
        cup.x_pos = (num_cups == 1) ? MAP_WIDTH / 2.0f : CUP_MARGIN + i * spacing;
        cup.target_x = cup.x_pos;
        cups.push_back(cup);
    }

    // Randomly assign which cup starts with the ball
    ball_cup_id = std::uniform_int_distribution<int>(0, num_cups - 1)(rng);
}

void ShellGame_MiniGame::reset_player_guesses()
{
    for (auto& [id, player] : players)
    {
        player.guessed_cup = -1;
        player.was_correct = false;
    }
}

int ShellGame_MiniGame::count_active_players() const
{
    int count = 0;
    for (const auto& [id, player] : players)
    {
        if (!player.eliminated) count++;
    }
    return count;
}

bool ShellGame_MiniGame::all_active_players_guessed() const
{
    for (const auto& [id, player] : players)
    {
        if (!player.eliminated && player.guessed_cup == -1) return false;
    }
    return true;
}

// ─── Input handling ──────────────────────────────────────────────────────────

void ShellGame_MiniGame::process_input(const MiniGamePayloadType* payload, Client* from)
{
    if (payload->gamestatetype() != GameStateType_ShellGamePlayerInput) return;

    if (current_phase != ShellGame_Phase::GUESS) return;

    auto it = players.find(from->client_id);
    if (it == players.end() || it->second.eliminated) return;
    if (it->second.guessed_cup != -1) return;  // already guessed

    auto input = payload->gamestatepayload_as_ShellGamePlayerInputPayload();
    if (input == nullptr) return;

    int chosen = (int)input->cup_index();
    if (chosen < 0 || chosen >= (int)cups.size()) return;

    it->second.guessed_cup = chosen;
    send_player_update(from->client_id);
}

// ─── FlatBuffer sending ──────────────────────────────────────────────────────

void ShellGame_MiniGame::send_host_update()
{
    flatbuffers::FlatBufferBuilder builder;

    // Build cups vector (must be created before the table that references it)
    std::vector<flatbuffers::Offset<FBShellGameCup>> cup_offsets;
    cup_offsets.reserve(cups.size());
    for (const auto& cup : cups)
    {
        cup_offsets.push_back(CreateFBShellGameCup(builder, cup.x_pos, cup.depth));
    }
    auto cups_vec = builder.CreateVector(cup_offsets);

    // Ball is only visible in REVEAL and ROUND_RESULT
    short ball_index = -1;
    if (current_phase == ShellGame_Phase::REVEAL || current_phase == ShellGame_Phase::ROUND_RESULT)
    {
        ball_index = (short)ball_cup_id;
    }

    auto payload = CreateShellGameHostPayload(builder,
        (ShellGamePhase)(int8_t)current_phase,
        (uint16_t)current_round,
        remaining_time,
        cups_vec,
        ball_index,
        (uint16_t)count_active_players(),
        MAP_WIDTH
    );

    auto game_name = builder.CreateString(get_camel_case_name());
    auto gs_payload = CreateMiniGamePayloadType(builder,
        game_name,
        GameStateType_ShellGameHost,
        GameStatePayload_ShellGameHostPayload,
        payload.Union()
    );

    game->party->send_gamestate(
        [](Client* c) { return c->party->host == c; },
        builder,
        gs_payload.Union()
    );
}

void ShellGame_MiniGame::send_player_update(int client_id)
{
    auto it = players.find(client_id);
    if (it == players.end()) return;

    const ShellGame_Player& player = it->second;

    flatbuffers::FlatBufferBuilder builder;

    auto payload = CreateShellGamePlayerPayload(builder,
        (int8_t)current_phase,
        (uint16_t)current_round,
        remaining_time,
        (uint16_t)num_cups,
        player.eliminated,
        player.guessed_cup != -1,
        player.was_correct
    );

    auto game_name = builder.CreateString(get_camel_case_name());
    auto gs_payload = CreateMiniGamePayloadType(builder,
        game_name,
        GameStateType_ShellGamePlayer,
        GameStatePayload_ShellGamePlayerPayload,
        payload.Union()
    );

    int target_id = client_id;
    game->party->send_gamestate(
        [target_id](Client* c) { return c->client_id == target_id; },
        builder,
        gs_payload.Union()
    );
}

void ShellGame_MiniGame::send_all_player_updates()
{
    for (const auto& [id, player] : players)
    {
        send_player_update(id);
    }
}

void ShellGame_MiniGame::send_round_result_to_host()
{
    flatbuffers::FlatBufferBuilder builder;

    // Build player results vector
    std::vector<flatbuffers::Offset<FBShellGameRoundPlayerResult>> result_offsets;
    result_offsets.reserve(players.size());
    for (const auto& [id, player] : players)
    {
        if (player.client == nullptr) continue;
        auto name = builder.CreateString(player.client->name);
        result_offsets.push_back(CreateFBShellGameRoundPlayerResult(builder,
            name,
            player.was_correct,
            (int16_t)player.guessed_cup
        ));
    }
    auto results_vec = builder.CreateVector(result_offsets);

    auto payload = CreateShellGameRoundResultPayload(builder,
        (uint16_t)current_round,
        (uint16_t)get_ball_position_index(),
        results_vec,
        (uint16_t)count_active_players()
    );

    auto game_name = builder.CreateString(get_camel_case_name());
    auto gs_payload = CreateMiniGamePayloadType(builder,
        game_name,
        GameStateType_ShellGameRoundResult,
        GameStatePayload_ShellGameRoundResultPayload,
        payload.Union()
    );

    game->party->send_gamestate(
        [](Client* c) { return c->party->host == c; },
        builder,
        gs_payload.Union()
    );
}

void ShellGame_MiniGame::send_result_data()
{
    auto result = getMinigameResult();

    flatbuffers::FlatBufferBuilder builder;

    std::vector<flatbuffers::Offset<FBShellGameResultPair>> pair_offsets;
    pair_offsets.reserve(result.size());

    for (const auto& [client, placement] : result)
    {
        if (client == nullptr) continue;
        auto it = players.find(client->client_id);
        int rounds_survived = (it != players.end() && it->second.finished_round > 0)
            ? it->second.finished_round
            : current_round;

        auto name = builder.CreateString(client->name);
        pair_offsets.push_back(CreateFBShellGameResultPair(builder,
            name,
            (uint16_t)placement,
            (uint16_t)rounds_survived
        ));
    }
    auto pairs_vec = builder.CreateVector(pair_offsets);

    auto payload = CreateShellGameResultPayload(builder, pairs_vec);

    auto game_name = builder.CreateString(get_camel_case_name());
    auto gs_payload = CreateMiniGamePayloadType(builder,
        game_name,
        GameStateType_ShellGameResult,
        GameStatePayload_ShellGameResultPayload,
        payload.Union()
    );

    game->party->send_gamestate(
        [](Client* c) { return true; },
        builder,
        gs_payload.Union()
    );
}

// ─── Results / metadata ──────────────────────────────────────────────────────

std::vector<std::pair<Client*, int>> ShellGame_MiniGame::getMinigameResult()
{
    // Sort players: those who lasted longer rank higher
    std::vector<ShellGame_Player*> sorted;
    sorted.reserve(players.size());
    for (auto& [id, player] : players)
    {
        sorted.push_back(&player);
    }

    std::sort(sorted.begin(), sorted.end(), [](const ShellGame_Player* a, const ShellGame_Player* b)
    {
        int ra = (a->finished_round == -1) ? INT_MAX : a->finished_round;
        int rb = (b->finished_round == -1) ? INT_MAX : b->finished_round;
        return ra > rb;
    });

    std::vector<std::pair<Client*, int>> results;
    results.reserve(sorted.size());

    int placement = 1;
    for (int i = 0; i < (int)sorted.size(); i++)
    {
        if (i > 0)
        {
            int prev_r = (sorted[i-1]->finished_round == -1) ? INT_MAX : sorted[i-1]->finished_round;
            int curr_r = (sorted[i]->finished_round == -1)   ? INT_MAX : sorted[i]->finished_round;
            if (curr_r != prev_r) placement = i + 1;
        }
        results.push_back({sorted[i]->client, placement});
    }

    return results;
}

std::string ShellGame_MiniGame::get_display_name() { return "Shell Game"; }
std::string ShellGame_MiniGame::get_camel_case_name() { return "shellGame"; }
std::string ShellGame_MiniGame::get_description() { return "Watch which cup hides the ball, then track it through the shuffle!"; }

void ShellGame_MiniGame::clients_changed(int client_id, bool joined) { }

void ShellGame_MiniGame::pause()
{
    introduction_timer.pause();
    timer.pause();
    result_timer.pause();
}

void ShellGame_MiniGame::resume()
{
    introduction_timer.resume();
    timer.resume();
    result_timer.resume();
}
