#include "teambased_pong_minigame.h"
#include "../../game.h"
#include <algorithm>
#include <random>

TeambasedPong_MiniGame::TeambasedPong_MiniGame(Game* game) : MiniGame(game) {
    min_players = 2;
    max_players = -1; // No limit

    delta_time = floor(1000 / target_fps);
}

TeambasedPong_MiniGame::~TeambasedPong_MiniGame() {
    introduction_timer.clear();
    minigame_timer.clear();
    result_timer.clear();
    
    if (map) {
        delete map;
        map = nullptr;
    }
}

void TeambasedPong_MiniGame::start_introduction() {
    send_minigame_introduction(
        get_camel_case_name(),
        introduction_time,
        get_display_name(),
        get_description()
    );
    
    introduction_timer.setInterval([this]() {
        introduction_update(500 MILLISECONDS);
    }, 500 MILLISECONDS);
}

void TeambasedPong_MiniGame::introduction_update(int delta_time) {
    // Introduction countdown logic
    introduction_time -= delta_time;
    
    if (introduction_time <= 0) {
        introduction_timer.clear();
        start_minigame();
    }

    send_minigame_introduction(get_camel_case_name(), introduction_time, get_display_name(), get_description());
}

void TeambasedPong_MiniGame::start_minigame() {
    // Create players from current game clients
    players.clear();
    active_players.clear();
    
    for (auto client : game->get_clients()) {
        if (!client->isHost) {
            TeambasedPong_Player player(client, Team::SPECTATOR);
            players[client->client_id] = player;
            active_players.push_back(client);
        }
    }
    
    // Prepare teams for the first round
    prepare_next_round_teams();
    
    // Start first round
    current_round = 0;
    start_new_round();
    
    // Start update loop
    minigame_timer.setInterval([this]() { update(delta_time); }, delta_time);
}

void TeambasedPong_MiniGame::start_new_round() {
    current_round++;
    remaining_round_time = round_time + preview_time; // Include preview time
    current_phase = PongPhase::PLAYING;
    
    // Use the pre-assigned next teams as current teams
    teamA_clients = next_teamA_clients;
    teamB_clients = next_teamB_clients;
    
    // Note: Team enums are already updated in prepare_next_round_teams()
    // No need to update them again here
    
    // Create/reset map
    if (map) {
        delete map;
    }
    map = new TeambasedPong_Map();
    map->Initialize();
    
    // Send round start message to clients
    send_host_update();
    for (auto client : game->get_clients()) {
        send_player_update(client);
    }
}

void TeambasedPong_MiniGame::prepare_next_round_teams() {
    next_teamA_clients.clear();
    next_teamB_clients.clear();
    
    // Shuffle active players
    std::vector<Client*> shuffled_players = active_players;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(shuffled_players.begin(), shuffled_players.end(), gen);
    
    // Split into two teams
    int half = shuffled_players.size() / 2;
    for (int i = 0; i < shuffled_players.size(); i++) {
        Client* client = shuffled_players[i];
        if (i < half) {
            next_teamA_clients.push_back(client);
            // Update player team enum immediately
            if (players.find(client->client_id) != players.end()) {
                players[client->client_id].team = Team::TEAM_A;
            }
        } else {
            next_teamB_clients.push_back(client);
            // Update player team enum immediately
            if (players.find(client->client_id) != players.end()) {
                players[client->client_id].team = Team::TEAM_B;
            }
        }
    }
}

void TeambasedPong_MiniGame::calculate_team_input() {
    // Calculate average input for each team
    float teamA_total = 0.0f;
    int teamA_count = 0;
    
    for (auto client : teamA_clients) {
        if (players.find(client->client_id) != players.end()) {
            teamA_total += players[client->client_id].joystick_y;
            teamA_count++;
        }
    }
    
    float teamB_total = 0.0f;
    int teamB_count = 0;
    
    for (auto client : teamB_clients) {
        if (players.find(client->client_id) != players.end()) {
            teamB_total += players[client->client_id].joystick_y;
            teamB_count++;
        }
    }
    
    // Apply average input to map
    if (map) {
        float teamA_input = teamA_count > 0 ? teamA_total / teamA_count : 0.0f;
        float teamB_input = teamB_count > 0 ? teamB_total / teamB_count : 0.0f;
        
        map->SetTeamInput(Team::TEAM_A, teamA_input);
        map->SetTeamInput(Team::TEAM_B, teamB_input);
    }
}

void TeambasedPong_MiniGame::update(int delta_time) {
    switch (current_phase) {
        case PongPhase::PLAYING:
            update_playing_phase(delta_time);
            break;
        case PongPhase::ROUND_RESULT:
            update_round_result_phase(delta_time);
            break;
    }
}

void TeambasedPong_MiniGame::update_playing_phase(int delta_time) {
    if (!map) return;
    
    // Update remaining time
    remaining_round_time -= delta_time;
    
    // Check if we're in preview phase (first 2 seconds)
    bool is_preview = remaining_round_time > round_time;
    
    if (!is_preview) {
        // Active gameplay - calculate input and update physics
        calculate_team_input();
        
        // Update physics
        float dt = delta_time / 1000.0f; // Convert to seconds
        map->Update(dt);
        
        // Check for scoring
        Team scoring_team = map->GetScoringTeam();
        if (scoring_team != Team::SPECTATOR) {
            end_current_round(scoring_team);
            return;
        }
    }
    
    // Check for timeout (tie) - only after preview
    if (remaining_round_time <= 0) {
        // Tie - both teams eliminated equally
        end_current_round(Team::SPECTATOR);
        return;
    }
    
    // Send updates to clients (in both preview and active gameplay)
    send_host_update();
    for (auto client : game->get_clients()) {
        if (!client->isHost) {
            send_player_update(client);
        }
    }
}

void TeambasedPong_MiniGame::update_round_result_phase(int delta_time) {
    // Update remaining time
    remaining_round_time -= delta_time;
    
    // Send updates to clients (helps when someone joins mid-game or on resume)
    send_round_result();
    
    // Check if round result time is over
    if (remaining_round_time <= 0) {
        // Check if game is over or continue to next round
        if (active_players.size() <= 1) {
            start_result();
        } else {
            start_new_round();
        }
    }
}

void TeambasedPong_MiniGame::end_current_round(Team winning_team) {
    current_phase = PongPhase::ROUND_RESULT;
    remaining_round_time = round_result_time; // Reset timer for round result phase
    last_round_winner = winning_team;
    
    // Update player match counts
    std::vector<Client*> winners;
    std::vector<Client*> losers;
    
    if (winning_team == Team::TEAM_A) {
        winners = teamA_clients;
        losers = teamB_clients;
    } else if (winning_team == Team::TEAM_B) {
        winners = teamB_clients;
        losers = teamA_clients;
    } else {
        // Tie - both teams eliminated
        losers = active_players;
    }
    
    // Increment winners' played_matches
    for (auto client : winners) {
        if (players.find(client->client_id) != players.end()) {
            players[client->client_id].played_matches++;
        }
    }
    
    // Set losers to SPECTATOR team and remove from active players
    for (auto client : losers) {
        if (players.find(client->client_id) != players.end()) {
            players[client->client_id].team = Team::SPECTATOR;
        }
    }
    
    active_players.erase(
        std::remove_if(active_players.begin(), active_players.end(),
            [&losers](Client* c) {
                return std::find(losers.begin(), losers.end(), c) != losers.end();
            }),
        active_players.end()
    );
    
    // Prepare teams for next round if game continues
    if (active_players.size() > 1) {
        prepare_next_round_teams();
    }
    
    // Send initial round result
    send_round_result();
}

void TeambasedPong_MiniGame::start_result() {
    minigame_timer.clear();
    
    // TODO: Send result data to clients
    for (auto client : game->get_clients()) {
        send_result_data(client->client_id);
    }
    
    result_timer.setTimeout([this]() {
        finished();
    }, result_time);
}

void TeambasedPong_MiniGame::pause() {
    introduction_timer.pause();
    minigame_timer.pause();
    result_timer.pause();
}

void TeambasedPong_MiniGame::resume() {
    introduction_timer.resume();
    minigame_timer.resume();
    result_timer.resume();
}

std::vector<std::pair<Client*, int>> TeambasedPong_MiniGame::getMinigameResult() {
    std::vector<std::pair<Client*, int>> results;
    
    // Convert players map to vector for sorting
    std::vector<TeambasedPong_Player> player_list;
    for (auto& pair : players) {
        player_list.push_back(pair.second);
    }
    
    // Sort by played_matches (descending)
    std::sort(player_list.begin(), player_list.end());
    
    // Assign placements (players with same matches get same placement)
    int current_placement = 1;
    int last_matches = -1;
    int players_with_same_placement = 0;
    
    for (auto& player : player_list) {
        if (player.played_matches != last_matches) {
            current_placement += players_with_same_placement;
            players_with_same_placement = 0;
            last_matches = player.played_matches;
        }
        
        results.push_back({player.client, current_placement});
        players_with_same_placement++;
    }
    
    return results;
}

void TeambasedPong_MiniGame::process_input(const MiniGamePayloadType* payload, Client* from) {
    if (!payload) return;
    
    // Only process input during PLAYING phase
    if (current_phase != PongPhase::PLAYING) return;
    
    // Handle joystick input
    switch(payload->gamestatetype()) {
        case GameStateType_JoystickData: {
            auto input = payload->gamestatepayload_as_JoystickDataPayload();
            
            if (input && players.find(from->client_id) != players.end()) {
                // Update player's joystick Y value (only Y axis needed for pong)
                // Invert Y axis to match expected controls
                players[from->client_id].joystick_y = -input->y_pos();
            }
            break;
        }
        case GameStateType_JoystickEvent: {
            auto input = payload->gamestatepayload_as_JoystickEventPayload();
            if (input && players.find(from->client_id) != players.end()) {
                // if there is a stop event, reset joystick Y value to 0
                if (input->event_type() == JoystickEventType_Stop) {
                    players[from->client_id].joystick_y = 0.0f;
                }
            }
            break;
        }
        default:
            break;
    }
}

void TeambasedPong_MiniGame::send_host_update() {
    if (!map) return;
    
    flatbuffers::FlatBufferBuilder builder;
    
    // Get ball position
    Vector2D ball_pos = map->GetBallPosition();
    
    // Get paddle positions
    Vector2D paddle_a_pos = map->GetPaddlePosition(Team::TEAM_A);
    Vector2D paddle_b_pos = map->GetPaddlePosition(Team::TEAM_B);
    
    // Build team A players
    std::vector<flatbuffers::Offset<FBTeamPlayer>> team_a_buffer;
    for (auto client : teamA_clients) {
        auto name = builder.CreateString(client->name);
        team_a_buffer.push_back(CreateFBTeamPlayer(builder, name));
    }
    auto team_a_vector = builder.CreateVector(team_a_buffer);
    
    // Build team B players
    std::vector<flatbuffers::Offset<FBTeamPlayer>> team_b_buffer;
    for (auto client : teamB_clients) {
        auto name = builder.CreateString(client->name);
        team_b_buffer.push_back(CreateFBTeamPlayer(builder, name));
    }
    auto team_b_vector = builder.CreateVector(team_b_buffer);
    
    // Create the host payload
    auto payload = CreateTeambasedPongHostPayload(
        builder,
        current_round,
        remaining_round_time,
        TeambasedPong_Map::MAP_WIDTH,
        TeambasedPong_Map::MAP_HEIGHT,
        ball_pos.x,
        ball_pos.y,
        paddle_a_pos.x,
        paddle_a_pos.y,
        paddle_b_pos.x,
        paddle_b_pos.y,
        TeambasedPong_Map::PADDLE_WIDTH,
        TeambasedPong_Map::PADDLE_HEIGHT,
        team_a_vector,
        team_b_vector
    );
    
    auto miniGame = builder.CreateString(get_camel_case_name());
    auto gameStatePayload = CreateMiniGamePayloadType(
        builder, 
        miniGame, 
        GameStateType_TeambasedPongHost,
        GameStatePayload_TeambasedPongHostPayload, 
        payload.Union()
    );
    
    // Send to host only
    game->party->send_gamestate([](Client* client) { return client->party->host == client; }, builder, gameStatePayload.Union());
}

void TeambasedPong_MiniGame::send_player_update(Client* client) {
    flatbuffers::FlatBufferBuilder builder;
    
    // Determine player's team and if they're still playing
    PongTeam player_team = PongTeam_SPECTATOR;
    bool is_still_playing = false;
    
    if (players.find(client->client_id) != players.end()) {
        TeambasedPong_Player& player = players[client->client_id];
        
        // Convert Team enum to PongTeam enum
        switch (player.team) {
            case Team::TEAM_A:
                player_team = PongTeam_TEAM_A;
                break;
            case Team::TEAM_B:
                player_team = PongTeam_TEAM_B;
                break;
            case Team::SPECTATOR:
                player_team = PongTeam_SPECTATOR;
                break;
        }
        
        // Check if player is in active_players list
        is_still_playing = std::find(active_players.begin(), active_players.end(), client) != active_players.end();
    }
    
    // Create the player payload
    auto payload = CreateTeambasedPongPlayerPayload(
        builder,
        current_round,
        remaining_round_time,
        player_team,
        is_still_playing
    );
    
    auto miniGame = builder.CreateString(get_camel_case_name());
    auto gameStatePayload = CreateMiniGamePayloadType(
        builder, 
        miniGame, 
        GameStateType_TeambasedPongPlayer,
        GameStatePayload_TeambasedPongPlayerPayload, 
        payload.Union()
    );
    
    // Send to this specific client
    game->party->send_gamestate([client](Client* c) { return c == client; }, builder, gameStatePayload.Union());
}

void TeambasedPong_MiniGame::send_round_result() {
    flatbuffers::FlatBufferBuilder builder;
    
    // Determine round winner type
    PongRoundWinner round_winner;
    std::string winning_player_name = "";
    
    if (active_players.size() == 1) {
        // Single winner - game over
        round_winner = PongRoundWinner_SINGLE_WINNER;
        winning_player_name = active_players[0]->name;
    } else if (last_round_winner == Team::TEAM_A) {
        round_winner = PongRoundWinner_TEAM_A_WON;
    } else if (last_round_winner == Team::TEAM_B) {
        round_winner = PongRoundWinner_TEAM_B_WON;
    } else {
        round_winner = PongRoundWinner_TIE;
    }
    
    // Build next team assignments
    std::vector<flatbuffers::Offset<FBNextTeamPlayer>> next_team_a_buffer;
    for (auto client : next_teamA_clients) {
        auto name = builder.CreateString(client->name);
        next_team_a_buffer.push_back(CreateFBNextTeamPlayer(builder, name));
    }
    auto next_team_a_vector = builder.CreateVector(next_team_a_buffer);
    
    std::vector<flatbuffers::Offset<FBNextTeamPlayer>> next_team_b_buffer;
    for (auto client : next_teamB_clients) {
        auto name = builder.CreateString(client->name);
        next_team_b_buffer.push_back(CreateFBNextTeamPlayer(builder, name));
    }
    auto next_team_b_vector = builder.CreateVector(next_team_b_buffer);
    
    // Create the round result payload
    auto winner_name = builder.CreateString(winning_player_name);
    bool has_next_round = active_players.size() > 1;
    bool is_first_round = current_round == 1;
    
    auto payload = CreateTeambasedPongRoundResultPayload(
        builder,
        round_winner,
        winner_name,
        round_result_time,
        is_first_round,
        has_next_round,
        next_team_a_vector,
        next_team_b_vector
    );
    
    auto miniGame = builder.CreateString(get_camel_case_name());
    auto gameStatePayload = CreateMiniGamePayloadType(
        builder, 
        miniGame, 
        GameStateType_TeambasedPongRoundResult,
        GameStatePayload_TeambasedPongRoundResultPayload, 
        payload.Union()
    );
    
    // Send to all clients
    game->party->send_gamestate([](Client* client) { return client == client; }, builder, gameStatePayload.Union());
}

void TeambasedPong_MiniGame::send_result_data(int client_id) {
    flatbuffers::FlatBufferBuilder builder;
    
    // Get the minigame results
    auto mini_game_result = getMinigameResult();
    
    // Build results vector
    std::vector<flatbuffers::Offset<FBTeambasedPongResultPair>> results_buffer;
    for (auto& result_pair : mini_game_result) {
        Client* client = result_pair.first;
        int placement = result_pair.second;
        int rounds_won = 0;
        
        // Get rounds won from player data
        if (players.find(client->client_id) != players.end()) {
            rounds_won = players[client->client_id].played_matches;
        }
        
        auto name = builder.CreateString(client->name);
        results_buffer.push_back(CreateFBTeambasedPongResultPair(builder, name, placement, rounds_won));
    }
    auto results_vector = builder.CreateVector(results_buffer);
    
    // Create the result payload
    auto payload = CreateTeambasedPongResultPayload(builder, results_vector);
    
    auto miniGame = builder.CreateString(get_camel_case_name());
    auto gameStatePayload = CreateMiniGamePayloadType(
        builder, 
        miniGame, 
        GameStateType_TeambasedPongResult,
        GameStatePayload_TeambasedPongResultPayload, 
        payload.Union()
    );
    
    // Send to all clients
    game->party->send_gamestate([](Client* client) { return client == client; }, builder, gameStatePayload.Union());
}
