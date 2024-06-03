#include "memorymixer_mini_game.h"
#include "../../game.h"
#include "../../globals.h"

MemoryMixer_MiniGame::MemoryMixer_MiniGame(Game* game) : MiniGame(game) {
    max_on_card = 2;
    grid_size = 4;
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
    remaining_time = 20 SECONDS;

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
        timer.clear();
        start_result();
    }

    send_grid();
}

void MemoryMixer_MiniGame::send_grid() {
    flatbuffers::FlatBufferBuilder builder;

    std::vector<flatbuffers::Offset<MemoryMixerGridRow>> grid_row_buffer;
    for (int i = 0; i < grid_size; i++) {
        std::vector<flatbuffers::Offset<MemoryMixerGridCell>> grid_cell_buffer;
        for (int j = 0; j < grid_size; j++) {
            auto icon = MemoryMixerIconType(grid[i][j]);
            // todo add amount of players that have clicked on this cell
            auto grid_cell = CreateMemoryMixerGridCell(builder, icon);
            grid_cell_buffer.push_back(grid_cell);
        }

        auto row_vector = builder.CreateVector(grid_cell_buffer);
        auto grid_row = CreateMemoryMixerGridRow(builder, row_vector);
        grid_row_buffer.push_back(grid_row);
    }

    auto grid_row_vector = builder.CreateVector(grid_row_buffer);

    auto payload = CreateMemoryMixerGridPayload(builder, remaining_time, grid_row_vector);

    auto miniGame = builder.CreateString("memoryMixer");

    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_CrazyCountingHostEntities,
                                                      GameStatePayload_CrazyCountingHostEntitiesPayload, payload.Union());

    // Send payload to client
    game->party->send_gamestate([](Client* client) { return true; }, builder, gameStatePayload.Union());
}

void MemoryMixer_MiniGame::process_input(const MiniGamePayloadType* payload, Client* from) {
    switch(payload->gamestatetype()) {
        case GameStateType_MemoryMixerPlayerInput: {
            auto input = payload->gamestatepayload_as_MemoryMixerPlayerInputPayload();

            if (input->x() < 0 || input->x() >= grid_size || input->y() < 0 || input->y() >= grid_size) {
                return;
            }

            MemoryMixer_Player& player = players[from->client_id];
            player.submitted_x = input->x();
            player.submitted_y = input->y();
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