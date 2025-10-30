#include "unscrambled_mini_game.h"
#include "../../game.h"

Unscrambled_MiniGame::Unscrambled_MiniGame(Game *game) : MiniGame(game) {
    combinations = Word_Combinations::get_random_word_combinations(max_rounds);

    // loop 0-max_rounds to set a random round_target for each round
    for (int i = 0; i < max_rounds; i++) {
        int target = rand() % 5; // random number between 0 and 4
        round_target.push_back(target);
    }
}

Unscrambled_MiniGame::~Unscrambled_MiniGame() {
    introduction_timer.clear();
    minigame_timer.clear();
    result_timer.clear();
    timer.clear();

    players.clear();
    combinations.clear();
    round_target.clear();
}

void Unscrambled_MiniGame::start_introduction() {
    update_interval = 500 MILLISECONDS;
    introduction_timer.setInterval([this]() { introduction_update(update_interval); }, update_interval);
}

void Unscrambled_MiniGame::pause() {
    minigame_timer.pause();
    result_timer.pause();
    timer.pause();
    introduction_timer.pause();
}

void Unscrambled_MiniGame::resume() {
    minigame_timer.resume();
    result_timer.resume();
    timer.resume();
    introduction_timer.resume();
}

void Unscrambled_MiniGame::introduction_update(int delta_time) {
    introduction_time -= delta_time;

    if (introduction_time <= 0) {
        introduction_timer.clear();
        start_minigame();
        return;
    }

    send_minigame_introduction(get_camel_case_name(), introduction_time, get_display_name(), get_description());
}

void Unscrambled_MiniGame::select_random_word() {
    if (current_round - 1 >= combinations.size()) {
        return; // out of bounds
    }
    // select the set with index of current_round - 1
    auto &word_set = combinations[current_round - 1];

    // word is the round_target of current round
    int word_index = round_target[current_round - 1];
    current_word = word_set[word_index];

    // scramble the word
    while (true) {
        // loop until 70% different
        std::string test_scramble = current_word;
        std::shuffle(test_scramble.begin(), test_scramble.end(), std::mt19937{std::random_device{}()});
        int diff_count = 0;
        for (size_t i = 0; i < current_word.size(); i++) {
            if (current_word[i] != test_scramble[i]) {
                diff_count++;
            }
        }
        if (diff_count >= current_word.size() * 0.7) {
            current_scrambled_word = test_scramble;
            break;
        }
    }
}

void Unscrambled_MiniGame::unscramble_word_step() {
    // if allready unscrambled, return
    if (current_word == current_scrambled_word) {
        return;
    }

    // set 1 character to the correct position evey function call
    for (size_t i = 0; i < current_word.size(); i++) {
        if (current_word[i] != current_scrambled_word[i]) {
            // find the character in the scrambled word that should be at position i
            char target_char = current_word[i];
            size_t target_index = current_scrambled_word.find(target_char, i);
            if (target_index != std::string::npos) {
                // swap the characters
                std::swap(current_scrambled_word[i], current_scrambled_word[target_index]);
            }
            break; // only one character per step
        }
    }
}

void Unscrambled_MiniGame::start_minigame() {
    time = round_time;
    current_round = 1;
    current_phase = 0;

    // add players to the game
    for (auto client : game->party->get_clients()) {
        if (!client->isHost) {
            players[client].score = 0;
            players[client].has_submitted = false;
            players[client].guess = -1;
            players[client].time_taken = 0;
            players[client].temp_time_taken = 0;
        }
    }

    select_random_word();

    minigame_timer.setInterval([this]() { update(100 MILLISECONDS); }, 100 MILLISECONDS);
}

void Unscrambled_MiniGame::update(int delta_time) {
    time -= delta_time;

    // if time is up, go to next round/phase
    if (time <= 0) {
        if (current_phase == 0 && current_round <= max_rounds) {
            // go to round result
            current_phase = 1;
            for (auto &player : players) {
                if (!player.second.has_submitted) {
                    // it has not submitted, time_taken is max
                    player.second.time_taken += round_time;
                    player.second.temp_time_taken = round_time;
                }
            }
            send_round_result_data(game->party->host->client_id);
            for (auto &player : players) {
                send_round_result_data(player.first->client_id);
            }
            // set timer for result display
            time = 5 SECONDS;
            return;
        } else {
            // go to next round
            current_round++;
            time = round_time;
            current_phase = 0;
            select_random_word();

            // reset player submission status
            for (auto &player : players) {
                player.second.has_submitted = false;
                player.second.guess = -1;
                player.second.temp_time_taken = 0;
            }
            return;
        }
    }

    // if phase is 1, just return (to wait for timer to finish)
    if (current_phase == 1) {
        return;
    }

    // unscramble phase
    // every 2 seconds, unscramble one letter
    if (time % (2 SECONDS) == 0) {
        unscramble_word_step();
    }

    if (current_round == max_rounds + 1) {
        minigame_timer.clear();
        start_result();
    }
    else {
        // send minigame data
        send_host_payload_data(game->party->host->client_id);
        for (auto &player : players) {
            send_player_payload_data(player.first->client_id);
        }
    }

    // if all players have submitted, go to next round/phase
    bool all_submitted = true;
    for (const auto &player : players) {
        if (!player.second.has_submitted) {
            all_submitted = false;
            break;
        }
    }
    if (all_submitted) {
        time = 0;
    }
}

void Unscrambled_MiniGame::process_input(const MiniGamePayloadType *payload, Client *from) {
    switch(payload->gamestatetype()) {
        case GameStateType_UnscrambledPlayerInput: {
            auto input_payload = payload->gamestatepayload_as_UnscrambledPlayerInputPayload();
            if (input_payload) {
                auto &player_data = players[from];
                if (!player_data.has_submitted) { // only process if not allready submitted
                    // check if the guess is correct
                    if (input_payload->guess() == round_target[current_round - 1]) {
                        // correct
                        player_data.score += 1; // increase score by 1
                    }
                    player_data.has_submitted = true; // mark as submitted
                    player_data.guess = input_payload->guess();
                    player_data.temp_time_taken = round_time - time;
                    player_data.time_taken += round_time - time;
                }
            }
            break;
        }
    }
}

void Unscrambled_MiniGame::send_host_payload_data(int client_id) {
    flatbuffers::FlatBufferBuilder builder;

    auto payload = CreateUnscrambledHostPayload(builder, time, current_round, builder.CreateString(current_scrambled_word));

    auto miniGame = builder.CreateString(get_camel_case_name());
    
    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_UnscrambledHost,
                                                      GameStatePayload_UnscrambledHostPayload, payload.Union());

    game->party->send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, builder, gameStatePayload.Union());
}

void Unscrambled_MiniGame::send_player_payload_data(int client_id) {
    flatbuffers::FlatBufferBuilder builder;

    // create vector of current word set
    std::vector<flatbuffers::Offset<flatbuffers::String>> word_vector;
    for (const auto &word : combinations[current_round - 1]) {
        word_vector.push_back(builder.CreateString(word));
    }
    auto words_vector = builder.CreateVector(word_vector);

    auto player = players[game->party->get_client(client_id)];

    auto payload = CreateUnscrambledPlayerPayload(builder, time, current_round, words_vector, player.has_submitted);

    auto miniGame = builder.CreateString(get_camel_case_name());
    
    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_UnscrambledPlayer,
                                                      GameStatePayload_UnscrambledPlayerPayload, payload.Union());

    game->party->send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, builder, gameStatePayload.Union());
}

void Unscrambled_MiniGame::send_round_result_data(int client_id) {
    flatbuffers::FlatBufferBuilder builder;

    // create vector of results
    std::vector<flatbuffers::Offset<FBUnscrambledRoundResultPair>> results_vector;
    for (const auto &player : players) {
        auto name = builder.CreateString(player.first->name);
        results_vector.push_back(CreateFBUnscrambledRoundResultPair(builder, name, player.second.score, player.second.guess, player.second.temp_time_taken));
    }
    auto resultsPayload = builder.CreateVector(results_vector);

    auto payload = CreateUnscrambledRoundResultPayload(builder, current_round, round_target[current_round - 1], resultsPayload);

    auto miniGame = builder.CreateString(get_camel_case_name());
    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_UnscrambledRoundResult,
                                                      GameStatePayload_UnscrambledRoundResultPayload, payload.Union());

    game->party->send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, builder, gameStatePayload.Union());
}

void Unscrambled_MiniGame::start_result() {
    send_result_data(game->party->host->client_id);
    for (auto &player : players) {
        send_result_data(player.first->client_id);
    }

    result_timer.setTimeout([this]() {
        finished();
    }, result_time);
}

void Unscrambled_MiniGame::send_result_data(int client_id) {
    flatbuffers::FlatBufferBuilder builder;

    // create vector of minigame results
    std::vector<flatbuffers::Offset<FBUnscrambledResultPair>> results_vector;
    auto minigame_results = getMinigameResult();
    for (const auto &result : minigame_results) {
        auto name = builder.CreateString(result.first->name);
        results_vector.push_back(CreateFBUnscrambledResultPair(builder, name, players[result.first].score, players[result.first].time_taken, result.second));
    }
    auto resultsPayload = builder.CreateVector(results_vector);

    auto payload = CreateUnscrambledResultPayload(builder, resultsPayload);

    auto miniGame = builder.CreateString(get_camel_case_name());
    
    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_UnscrambledResult,
                                                      GameStatePayload_UnscrambledResultPayload, payload.Union());

    game->party->send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, builder, gameStatePayload.Union());
}

std::vector<std::pair<Client *, int>> Unscrambled_MiniGame::getMinigameResult() {
    std::vector<std::pair<Client *, Unscrampled_player_data>> local_players;
    for (auto &player : this->players) {
        local_players.push_back(player);
    }

    sort(local_players.begin(), local_players.end(), [&](const std::pair<Client *, Unscrampled_player_data> &a, const std::pair<Client *, Unscrampled_player_data> &b) {
        if (a.second.score == b.second.score)  {
            if (a.second.time_taken == b.second.time_taken) {
                return true;
            }
            return a.second.time_taken < b.second.time_taken;
        }
        return b.second.score < a.second.score;
    });


    // give placement to players (players can have the same placement)
    std::vector<std::pair<Client *, int>> result;
    for (int i = 0; i < local_players.size(); i++) {
        // if the score and time_taken of the previous player is the same, give the same placement as previous player
        if (i != 0 && local_players[i].second.score == local_players[i - 1].second.score && local_players[i].second.time_taken == local_players[i - 1].second.time_taken) {
            int previous_placement = result[i - 1].second;
            result.push_back(std::make_pair(local_players[i].first, previous_placement));
        } else {
            result.push_back(std::make_pair(local_players[i].first, i + 1));
        }
    }
    return result;
}