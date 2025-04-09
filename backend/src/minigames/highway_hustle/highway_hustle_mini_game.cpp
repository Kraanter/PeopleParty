#include "highway_hustle_mini_game.h"

HighwayHustle_MiniGame::HighwayHustle_MiniGame(Game *game) : MiniGame(game) {
    map = new HighwayHustle_Map();
}

HighwayHustle_MiniGame::~HighwayHustle_MiniGame() {
    introduction_timer.clear();
    minigame_timer.clear();
    result_timer.clear();
    delete map;
}

void HighwayHustle_MiniGame::start_introduction() {
    update_interval = 500 MILLISECONDS;
    introduction_timer.setInterval([this]() { introduction_update(update_interval); }, update_interval);
}

void HighwayHustle_MiniGame::pause() {
    minigame_timer.pause();
    result_timer.pause();
    introduction_timer.pause();
}

void HighwayHustle_MiniGame::resume() {
    minigame_timer.resume();
    result_timer.resume();
    introduction_timer.resume();

}

void HighwayHustle_MiniGame::introduction_update(int delta_time)
{
    introduction_time -= delta_time;

    if (introduction_time <= 0) {
        introduction_timer.clear();
        start_minigame();
        return;
    }

    send_minigame_introduction(get_camel_case_name(), introduction_time, get_display_name(), get_description());
}

void RightOnTime_Minigame::start_minigame() {
    map->create_players(game->get_clients());

    minigame_timer.setInterval([this]() { update(100 MILLISECONDS); }, 100 MILLISECONDS);
}

void HighwayHustle_MiniGame::process_input(const MiniGamePayloadType *payload, Client *from) {
    // decode the payload

    //map->update_player_velocity(from, x, y);
}
