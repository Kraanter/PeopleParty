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
        if (!player->isHost) {
            Moving_Entity* entity = new Moving_Entity();
    
            // todo: give players a position depending on the ammount of players allready added
    
            this->players.insert(std::pair<Client*, Moving_Entity*>(player, entity));
        }
    }
}

void HighwayHustle_Map::update_player_velocity(Client *player, float x, float y) {
    if (players.find(player) != players.end()) {
        Vector2D velocity = Vector2D(x, y);

        players[player]->add_velocity(velocity);
    }
}

void HighwayHustle_Map::update_player_event(Client *player, int event_type) {
    if (players.find(player) != players.end()) {
        if (event_type == 0) { // stop
            players[player]->change_joystick_is_moving(false);
        } else if (event_type == 1) { // start
            players[player]->change_joystick_is_moving(true);
        }
    }
}


void HighwayHustle_Map::update(unsigned long delta_time) {
    // update players
    for (auto &player : players) {
        player.second->update(delta_time);

        if (player.second->position.x < 0 && !player.second->is_dead) {
            player.second->position.x = 0;
        }
        if (player.second->position.x > map_width) {
            player.second->position.x = map_width;
        }
        if (player.second->position.y < 0) {
            player.second->position.y = 0;
        }
        if (player.second->position.y > map_height) {
            player.second->position.y = map_height;
        }
    }

    // update obstacles
    update_obstacles(delta_time);

    // check collisions between players and obstacles
    for (auto &player : players) {
        if (player.second->is_dead) {
            continue;
        }
        for (auto &obstacle : obstacles) {
            if (player.second->check_colision(obstacle)) {
                player.second->is_dead = true;
                player.second->velocity = obstacle->velocity;
                player.second->final_score = distance_travelled / 10;
                break;
            }
        }
    }

    // update distance travelled
    distance_travelled += (speed * (delta_time / 10) / 2);
    speed *= 1.0011f;
}

void HighwayHustle_Map::update_obstacles(unsigned long delta_time) {
    // update obstacles
    for (auto &obstacle : obstacles) {
        obstacle->update(delta_time);
    }

    // create new obstacles
    if (obstacles.size() < 50) {
        // TODO: ramp up when distance is higher;
        if ((float)std::rand() / RAND_MAX < 0.005f) {
            create_obstacle();
        }
    }

    // delete obstacle if off screen (left side)
    auto it = obstacles.begin();
    while (it != obstacles.end()) {
        if ((*it)->position.x < -(*it)->width) { 
            delete *it; it = obstacles.erase(it); 
        } else {
            it++;
        }
    }
}

void HighwayHustle_Map::create_obstacle() {
    // give random Y position
    int random_index = std::rand() % y_positions.size();
    int random_y = y_positions[random_index];

    Vector2D position = Vector2D(map_width + 5, random_y);
    Vector2D velocity = Vector2D(-0.01f * (std::max(2, distance_travelled / 1000)), 0);
    Obstacle_Entity* obstacle = new Obstacle_Entity(position, velocity);

    obstacle->id = std::to_string(obstacle_auto_increment);
    obstacle_auto_increment++;

    obstacles.push_back(obstacle);
}