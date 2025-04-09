#include "highway_hustle_map.h"

HighwayHustle_Map::HighwayHustle_Map(float x, float y) {
    map_height = y;
    map_width = x;
}

HighwayHustle_Map::~HighwayHustle_Map() {
    for (auto &player : players) {
        delete player.second;
    }
    players.clear();
}

void HighwayHustle_Map::create_players(std::vector<Client *> players) {
    for (auto &player : players) {
        Moving_Entity* entity = new Moving_Entity();

        // todo: give players a position depending on the ammount of players allready added

        this->players.insert(std::pair<Client*, Moving_Entity*>(player, entity));
    }
}

void HighwayHustle_Map::update_player_velocity(Client *player, float x, float y) {
    if (players.find(player) != players.end()) {
        Vector2D velocity = Vector2D(x, y);

        players[player]->velocity += velocity;
        players[player]->velocity.Truncate(0.1f);
    }
}


void HighwayHustle_Map::update(unsigned long delta_time) {
    for (auto &player : players) {
        player.second->update(delta_time);
    }
}