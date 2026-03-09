#ifndef PEOPLEPARTY_BACKEND_TEAMBASED_PONG_MINIGAME_H
#define PEOPLEPARTY_BACKEND_TEAMBASED_PONG_MINIGAME_H

#include "../../flatbuffer/messageClass_generated.h"
#include "../minigame.h"
#include <vector>
#include <map>
#include "teambased_pong_enums.h"
#include "teambased_pong_player.h"
#include "teambased_pong_map.h"

// Forward declare
class TeambasedPong_Map;
class TeambasedPong_Player;

class TeambasedPong_MiniGame : public MiniGame {
private:
    int target_fps = 20;
    int result_time = 10 SECONDS;
    int preview_time = 5 SECONDS; // Preview time before game starts
    int round_time = 60 SECONDS; // Max time per round before tie (excluding preview)
    int round_result_time = 10 SECONDS; // Time to show round results
    int delta_time = 0; // update interval, set in constructor
    
    ThreadTimer introduction_timer;
    ThreadTimer minigame_timer;
    ThreadTimer result_timer;
    
    // Game state
    PongPhase current_phase = PongPhase::PLAYING;
    TeambasedPong_Map* map = nullptr;
    std::map<int, TeambasedPong_Player> players; // client_id -> player
    std::vector<Client*> active_players; // Players still competing
    int current_round = 0;
    int remaining_round_time = 0;
    
    // Track team assignments for current round
    std::vector<Client*> teamA_clients;
    std::vector<Client*> teamB_clients;
    
    // Track team assignments for NEXT round (shown during round result)
    std::vector<Client*> next_teamA_clients;
    std::vector<Client*> next_teamB_clients;
    
    // Track last round results
    Team last_round_winner = Team::SPECTATOR;
    
public:
    TeambasedPong_MiniGame(Game* game);
    ~TeambasedPong_MiniGame();
    
    void start_introduction() override;
    void start_minigame() override;
    void start_result() override;
    void pause() override;
    void resume() override;
    std::vector<std::pair<Client *, int>> getMinigameResult() override;
    void clients_changed(int client_id, bool joined) override { };
    
    std::string get_display_name() override { return "Team-Based Pong"; };
    std::string get_camel_case_name() override { return "teambasedPong"; };
    std::string get_description() override { return "Work together to control your paddle and defeat the other team!"; };
    
private:
    void update(int delta_time) override;
    void process_input(const MiniGamePayloadType* payload, Client* from) override;
    void introduction_update(int delta_time);
    
    // Phase management
    void update_playing_phase(int delta_time);
    void update_round_result_phase(int delta_time);
    
    // Round management
    void start_new_round();
    void end_current_round(Team winning_team);
    void prepare_next_round_teams(); // Assigns teams for NEXT round
    void calculate_team_input();
    
    // Sending data (placeholder for now)
    void send_host_update();
    void send_player_update(Client* client);
    void send_result_data(int client_id);
    void send_round_result(); // Shows who won and next teams
};

#endif //PEOPLEPARTY_BACKEND_TEAMBASED_PONG_MINIGAME_H
