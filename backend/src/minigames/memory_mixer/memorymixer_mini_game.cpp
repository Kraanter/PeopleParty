#include "memorymixer_mini_game.h"
#include "../../game.h"
#include "../../globals.h"

MemoryMixer_MiniGame::MemoryMixer_MiniGame(Game* game) : MiniGame(game) {
    grid_size = 5;
    unique_symbols = 3;

    active_players = -1;

    std::vector<Client*> clients = game->get_clients();
    for (Client* client : clients) {
        if (client->isHost) {
            host = client;
        }
    }
}

MemoryMixer_MiniGame::~MemoryMixer_MiniGame() {
    introduction_timer.clear();
    timer.clear();
    results_timer.clear();
}

std::string MemoryMixer_MiniGame::get_display_name() {
    return "Memory Mixer";
}

std::string MemoryMixer_MiniGame::get_camel_case_name() {
    return "memoryMixer";
}

std::string MemoryMixer_MiniGame::get_description() {
    return "Click the correct icon on your phone to win!";
}

void MemoryMixer_MiniGame::create_grid() {
    target_card = MemoryMixer_card(rand() % (unique_symbols));

    int correct_cards = 0;
    std::vector<MemoryMixer_card> card_list;

    for (int i = 0; i < unique_symbols; i++) {
        for (int j = 0; j < max_correct; j++) {
            card_list.push_back(MemoryMixer_card(i));
        }
    }

    while (card_list.size() < grid_size * grid_size) {
        int rnd = rand() % (unique_symbols);
        while(rnd == target_card) {
            rnd = rand() % (unique_symbols);
        }
        card_list.push_back(MemoryMixer_card(rnd));
    }

    auto rd = std::random_device {}; 
    auto rng = std::default_random_engine { rd() };
    std::shuffle(std::begin(card_list), std::end(card_list), rng);
    
    grid = std::vector<std::vector<MemoryMixer_card>>(grid_size, std::vector<MemoryMixer_card>(grid_size));
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            grid[i][j] = card_list[i * 5 + j];
        }
    }
}

void MemoryMixer_MiniGame::set_round_difficulty(int current_round, double amount_of_players) {
    // max 100% of players can go to round 2
    // max 75% of players to round 3
    // max 75% of players to round 4
    // max 50% of players to round 5
    // max 25% of players to round 6
    if (current_round == 1) {
        unique_symbols = 3;
    } else if (current_round == 2) {
        unique_symbols = 4;
    } else if (current_round == 3) {
        unique_symbols = 5;
    } else if (current_round == 4) {
        unique_symbols = 6;
    } else if (current_round >= 5) {
        unique_symbols = 7;
    }

    max_correct = (grid_size * grid_size) / unique_symbols;

    if (amount_of_players > (max_correct / unique_symbols)) {
        max_on_card = std::ceil(amount_of_players / (max_correct / unique_symbols));
        if (max_on_card < 1) {
            max_on_card = 1;
        }
        
    } else {
        max_on_card = 1;
    }

    if ((current_round == 1 || current_round == 2) && max_on_card < 2) {
        max_on_card = 2;
    }
}

void MemoryMixer_MiniGame::introduction_update(int delta_time) {
    remaining_time -= delta_time;

    if(remaining_time <= 0) {
        introduction_timer.clear();
        start_minigame();
        return;
    }

    send_minigame_introduction(get_camel_case_name(), remaining_time, get_display_name(), get_description());
}

void MemoryMixer_MiniGame::start_introduction() {
    update_interval = 500 MILLISECONDS;
    remaining_time = introduction_time;

    introduction_timer.setInterval([this]() { introduction_update(update_interval); }, update_interval);
}

void MemoryMixer_MiniGame::start_minigame() {
    update_interval = 300 MILLISECONDS;
    remaining_time = memorise_time;
    round = 1;
    mini_game_phase = 0;

    for (Client* client : game->get_clients()) {
        if (client->isHost) {
            continue;
        }
        players[client->client_id] = MemoryMixer_Player(client->client_id);
    }

    active_players = players.size();
    set_round_difficulty(round, players.size());

    create_grid();

    timer.setInterval([this]() { update(update_interval); }, update_interval);
}

void MemoryMixer_MiniGame::start_result() {
    timer.clear();

    send_results(host->client_id);
    for (auto& [_, player] : players)
    {
        send_results(player.client_id);
    }

    results_timer.setTimeout([this]() {
        finished();
    }, 5 SECONDS);
}

void MemoryMixer_MiniGame::send_round_result(int client_id, std::vector<std::string> wrong_names, std::vector<std::string> correct_names) {
    flatbuffers::FlatBufferBuilder builder;

    std::vector<flatbuffers::Offset<flatbuffers::String>> correct_names_buffer;
    std::vector<flatbuffers::Offset<flatbuffers::String>> wrong_names_buffer;

    for (std::string name : correct_names) {
        correct_names_buffer.push_back(builder.CreateString(name));
    }
    for (std::string name : wrong_names) {
        wrong_names_buffer.push_back(builder.CreateString(name));
    }

    auto correct_names_vector = builder.CreateVector(correct_names_buffer);
    auto wrong_names_vector = builder.CreateVector(wrong_names_buffer);

    auto payload = CreateMemoryMixerRoundResultPayload(builder, round, correct_names_vector, wrong_names_vector);

    auto miniGame = builder.CreateString(get_camel_case_name());

    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_MemoryMixerRoundResult,
                                                      GameStatePayload_MemoryMixerRoundResultPayload, payload.Union());


    game->party->send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, builder, gameStatePayload.Union());
}

void MemoryMixer_MiniGame::next_round() {
    mini_game_phase = 0;
    remaining_time = memorise_time;

    // calculate round score
    std::vector<std::string> correct_names_buffer;
    std::vector<std::string> wrong_names_buffer;

    for (auto& [_, player] : players) {
        if (player.eliminated) {
            continue;
        }
        if (player.submitted_x == -1 || player.submitted_y == -1) {
            wrong_names_buffer.push_back(client_repository[player.client_id]->name);
            player.finished_round = round - 1;
            player.eliminated = true;
        } else if (grid[player.submitted_x][player.submitted_y] == target_card) {
            correct_names_buffer.push_back(client_repository[player.client_id]->name);
            player.finished_round = round;
        } else {
            wrong_names_buffer.push_back(client_repository[player.client_id]->name);
            player.finished_round = round - 1;
            player.eliminated = true;
        }
    }

    send_round_result(host->client_id, wrong_names_buffer, correct_names_buffer);
    for (auto& [_, player] : players) {
        send_round_result(player.client_id, wrong_names_buffer, correct_names_buffer);
    }

    int current_players = 0;
    for (auto& [_, player] : players) {
        player.submitted_x = -1;
        player.submitted_y = -1;
        send_player_submitted(player.client_id, false);
        if (!player.eliminated) {
            current_players++;
        }
    }
    active_players = current_players;

    timer.pause(round_results_time);
    
    if (active_players <= -1) {
        start_result();
        return;
    }

    round++;

    set_round_difficulty(round, current_players);

    create_grid();
}

void MemoryMixer_MiniGame::update(int delta_time) {
    remaining_time -= delta_time;

    if (remaining_time <= 0) {
        if (mini_game_phase == 0) {
            mini_game_phase = 1;
            remaining_time = guess_time;
        } else if (mini_game_phase == 1) {
            mini_game_phase = 2;
            remaining_time = 0;
        } else if (mini_game_phase == 2) {
            next_round();
            // timer.clear();
            // start_result();
            return;
        }
    }

    if (mini_game_phase == 0 || mini_game_phase == 1) {
        send_grid(host->client_id);
        for (auto& [_, player] : players) {
            if (!player.eliminated) {
                send_grid(player.client_id);
            }
        }
    } else if (mini_game_phase == 2) {
        send_grid(host->client_id, true);
        for (auto& [_, player] : players) {
            if (!player.eliminated) {
                send_grid(player.client_id, true);
            }
        }
    }
}

void MemoryMixer_MiniGame::send_grid(int client_id, bool highlight_correct) {
    if (mini_game_phase == 0 || mini_game_phase == 2) {
        flatbuffers::FlatBufferBuilder builder;

        auto gameStatePayload = build_grid(builder, mini_game_phase, false, highlight_correct);

        // Send payload to client
        game->party->send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, builder, gameStatePayload.Union());
    } else if (mini_game_phase == 1) {
        Client* client = client_repository[client_id];
        if (client->isHost) {
            flatbuffers::FlatBufferBuilder builder;
            auto gameStatePayloadHost = build_grid(builder, mini_game_phase, true, highlight_correct);
            game->party->send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, builder, gameStatePayloadHost.Union());
        } else {
            flatbuffers::FlatBufferBuilder builder;
            auto gameStatePayloadPlayer = build_grid(builder, mini_game_phase, false, highlight_correct);
            game->party->send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, builder, gameStatePayloadPlayer.Union());
        }
    }
}

flatbuffers::Offset<MiniGamePayloadType> MemoryMixer_MiniGame::build_grid(flatbuffers::FlatBufferBuilder &builder, int phase, bool to_host, bool highlight_correct) {
    int local_grid_size = grid_size;

    if (phase == 1) {
        if (to_host) {
            local_grid_size = 1;
        }
    }

    std::vector<flatbuffers::Offset<MemoryMixerGridRow>> grid_row_buffer;
    for (int i = 0; i < local_grid_size; i++) {
        std::vector<flatbuffers::Offset<MemoryMixerGridCell>> grid_cell_buffer;
        for (int j = 0; j < local_grid_size; j++) {
            int amount = 0;
            for (auto& [_, player] : players) {
                if (player.submitted_x == i && player.submitted_y == j) {
                    amount++;
                }
            }

            MemoryMixerIconType icon;
            if (phase == 0 || phase == 2) {
                icon = MemoryMixerIconType(grid[i][j]);
                amount = -1;
            } else if (phase == 1) {
                if (to_host) {
                    icon = MemoryMixerIconType(target_card);
                    amount = -1;
                } else {
                    icon = MemoryMixerIconType::MemoryMixerIconType_EMPTY;
                }
            }

            bool high_light = false;
            if (highlight_correct && grid[i][j] == target_card) {
                high_light = true;
            }

            auto grid_cell = CreateMemoryMixerGridCell(builder, icon, amount, high_light);
            grid_cell_buffer.push_back(grid_cell);
        }

        auto row_vector = builder.CreateVector(grid_cell_buffer);
        auto grid_row = CreateMemoryMixerGridRow(builder, row_vector);
        grid_row_buffer.push_back(grid_row);
    }

    auto grid_row_vector = builder.CreateVector(grid_row_buffer);

    std::vector<flatbuffers::Offset<flatbuffers::String>> submitted_names_buffer;
    for (auto& [_, player] : players) {
        if (player.submitted_x != -1 && player.submitted_y != -1) {
            submitted_names_buffer.push_back(builder.CreateString(client_repository[player.client_id]->name));
        }
    }
    auto submitted_names = builder.CreateVector(submitted_names_buffer);
    
    auto payload = CreateMemoryMixerGridPayload(builder, remaining_time, max_on_card, phase, round, active_players, submitted_names, grid_row_vector);

    auto miniGame = builder.CreateString(get_camel_case_name());

    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_MemoryMixerGrid,
                                                    GameStatePayload_MemoryMixerGridPayload, payload.Union());

    return gameStatePayload;
}

void MemoryMixer_MiniGame::send_player_submitted(int client_id, bool submitted) {
    flatbuffers::FlatBufferBuilder builder;

    auto player = players[client_id];
    auto payload = CreateMemoryMixerPlayerSubmittedPayload(builder, submitted, player.submitted_x, player.submitted_y);

    auto miniGame = builder.CreateString(get_camel_case_name());

    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_MemoryMixerPlayerSubmitted,
                                                      GameStatePayload_MemoryMixerPlayerSubmittedPayload, payload.Union());

    // Send payload to client
    game->party->send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, builder, gameStatePayload.Union());
}

void MemoryMixer_MiniGame::send_results(int client_id) {
    flatbuffers::FlatBufferBuilder builder;

    std::vector<Client*> clientRanking = getMinigameResult();

    std::vector<flatbuffers::Offset<FBMemoryMixerResultPair>> result_pair_buffer;
    for (int i = 0; i < clientRanking.size(); i++) {
        auto player = players[clientRanking[i]->client_id];

        auto result_pair = CreateFBMemoryMixerResultPair(builder, i + 1, builder.CreateString(clientRanking[i]->name), player.finished_round);

        result_pair_buffer.push_back(result_pair);
    }
    auto result_pair = builder.CreateVector(result_pair_buffer);

    auto payload = CreateMemoryMixerResultPayload(builder, round, result_pair);

    auto miniGame = builder.CreateString(get_camel_case_name());

    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_MemoryMixerResult,
                                                      GameStatePayload_MemoryMixerResultPayload, payload.Union());

    game->party->send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, builder, gameStatePayload.Union());

}

void MemoryMixer_MiniGame::process_input(const MiniGamePayloadType* payload, Client* from) {
    if (mini_game_phase != 1) {
        return;
    }

    switch(payload->gamestatetype()) {
        case GameStateType_MemoryMixerPlayerInput: {
            auto input = payload->gamestatepayload_as_MemoryMixerPlayerInputPayload();

            if (input->x() < 0 || input->x() >= grid_size || input->y() < 0 || input->y() >= grid_size) {
                return;
            }

            int amount = 0;
            for (auto& [_, player] : players) {
                if (player.eliminated) {
                    continue;
                }
                if (player.submitted_x == input->x() && player.submitted_y == input->y()) {
                    amount++;
                }
            }
            if (amount > max_on_card) {
                return;
            }

            MemoryMixer_Player& player = players[from->client_id];
            player.submitted_x = input->x();
            player.submitted_y = input->y();
            player.submitted_time = std::chrono::system_clock::now().time_since_epoch().count();
            send_player_submitted(from->client_id, true);
        }
    }
}

void MemoryMixer_MiniGame::clients_changed(int client_id, bool joined) {
    if (!joined && players.find(client_id) != players.end()) {
        players.erase(client_id);
    }
}

std::vector<Client*> MemoryMixer_MiniGame::getMinigameResult() {
    std::vector<MemoryMixer_Player> sorted_players;
    for (auto& [_, player] : players) {
        sorted_players.push_back(player);
    }

    std::vector<MemoryMixer_Player> correct_players;
    std::vector<MemoryMixer_Player> wrong_players;
    for (MemoryMixer_Player player: sorted_players) {
        if (player.submitted_x == -1 || player.submitted_y == -1) {
            wrong_players.push_back(player);
        } else if (grid[player.submitted_x][player.submitted_y] == target_card) {
            correct_players.push_back(player);
        } else {
            wrong_players.push_back(player);
        }
    }

    std::sort(correct_players.begin(), correct_players.end());
    std::sort(wrong_players.begin(), wrong_players.end());

    std::vector<Client*> result;
    for (MemoryMixer_Player player: correct_players) {
        result.push_back(client_repository[player.client_id]);
    }
    for (MemoryMixer_Player player: wrong_players) {
        result.push_back(client_repository[player.client_id]);
    }

    return result;
}