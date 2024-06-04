#include "memorymixer_mini_game.h"
#include "../../game.h"
#include "../../globals.h"

MemoryMixer_MiniGame::MemoryMixer_MiniGame(Game* game) : MiniGame(game) {
    max_on_card = 1;
    grid_size = 5;
    grid = std::vector<std::vector<MemoryMixer_card>>(grid_size, std::vector<MemoryMixer_card>(grid_size));
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            int rnd = rand() % (4 + 1);
            grid[i][j] = MemoryMixer_card(rnd);
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
    remaining_time = 10 SECONDS;
    mini_game_phase = 0;

    for (Client* client : game->get_clients()) {
        if (client->isHost) {
            continue;
        }
        players[client->client_id] = MemoryMixer_Player(client->client_id);
    }

    timer.setInterval([this]() { update(update_interval); }, update_interval);
}

void MemoryMixer_MiniGame::start_result() {
    // todo: implement
    results_timer.setTimeout([this]() { finished(); }, 5 SECONDS);
}

void MemoryMixer_MiniGame::update(int delta_time) {
    remaining_time -= delta_time;

    if (remaining_time <= 0) {
        if (mini_game_phase == 0) {
            mini_game_phase = 1;
            remaining_time = 15 SECONDS;
        } else if (mini_game_phase == 1) {
            mini_game_phase = 2;
            remaining_time = 5 SECONDS;
        } else if (mini_game_phase == 2) {
            mini_game_phase = 0;
            //timer.clear();
            //start_result();
            // fixme: revert this
            remaining_time = 30 SECONDS;
        }
    }

    send_grid();
}

void MemoryMixer_MiniGame::send_grid() {
    flatbuffers::FlatBufferBuilder builder;

    int local_grid_size = grid_size;

    if (mini_game_phase == 1) {
        local_grid_size = 1;
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
            if (mini_game_phase == 0) {
                icon = MemoryMixerIconType(grid[i][j]);
                amount = -1;
            } else if (mini_game_phase == 1) {
                int rnd = rand() % (4 + 1);
                icon = MemoryMixerIconType(rnd);
                amount = -1;
            } else if (mini_game_phase == 2) {
                icon = MemoryMixerIconType::MemoryMixerIconType_EMPTY;
            }

            auto grid_cell = CreateMemoryMixerGridCell(builder, icon, amount);
            grid_cell_buffer.push_back(grid_cell);
        }

        auto row_vector = builder.CreateVector(grid_cell_buffer);
        auto grid_row = CreateMemoryMixerGridRow(builder, row_vector);
        grid_row_buffer.push_back(grid_row);
    }

    auto grid_row_vector = builder.CreateVector(grid_row_buffer);

    auto payload = CreateMemoryMixerGridPayload(builder, remaining_time, max_on_card, grid_row_vector);

    auto miniGame = builder.CreateString("memoryMixer");

    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_MemoryMixerGrid,
                                                      GameStatePayload_MemoryMixerGridPayload, payload.Union());

    // Send payload to client
    game->party->send_gamestate([](Client* client) { return true; }, builder, gameStatePayload.Union());
}

void MemoryMixer_MiniGame::send_player_submitted(int client_id) {
    flatbuffers::FlatBufferBuilder builder;

    auto player = players[client_id];
    auto payload = CreateMemoryMixerPlayerSubmittedPayload(builder, true, player.submitted_x, player.submitted_y);

    auto miniGame = builder.CreateString("memoryMixer");

    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_MemoryMixerPlayerSubmitted,
                                                      GameStatePayload_MemoryMixerPlayerSubmittedPayload, payload.Union());

    // Send payload to client
    game->party->send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, builder, gameStatePayload.Union());
}

void MemoryMixer_MiniGame::process_input(const MiniGamePayloadType* payload, Client* from) {
    switch(payload->gamestatetype()) {
        case GameStateType_MemoryMixerPlayerInput: {
            auto input = payload->gamestatepayload_as_MemoryMixerPlayerInputPayload();

            if (input->x() < 0 || input->x() >= grid_size || input->y() < 0 || input->y() >= grid_size) {
                return;
            }

            int amount = 0;
            for (auto& [_, player] : players) {
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
            send_player_submitted(from->client_id);
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

    std::vector<Client*> result;
    for (MemoryMixer_Player player: sorted_players) {
        result.push_back(client_repository[player.client_id]);
    }

    return result;
}