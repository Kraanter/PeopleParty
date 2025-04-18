#ifndef PEOPLEPARTY_BACKEND_HIGHWAY_HUSTLE_MAP_H
#define PEOPLEPARTY_BACKEND_HIGHWAY_HUSTLE_MAP_H

#include <map>
#include <vector>
#include <cstdlib>
#include "moving_entity.h"
#include "obstacle_entity.h"
#include "../../client.h"

class Client;

class HighwayHustle_Map {
private:
    int map_height;
    int map_width;
    int distance_travelled = 0; // the distance travelled by the map
    float speed = 1.0f; // the speed of the map, ramps slowly up
public:
    std::map<Client*, Moving_Entity*> players;
    std::vector<Obstacle_Entity*> obstacles;
public:
    HighwayHustle_Map() { HighwayHustle_Map(0, 0); };
    HighwayHustle_Map(float x, float y);
    ~HighwayHustle_Map();
    void update(unsigned long delta_time);
    void create_players(std::vector<Client *> players);
    void update_player_velocity(Client* player, float x, float y);
    void update_player_event(Client* player, int event_type); // can be stop (0) or start (1), joystick events
    //void update_obstacle(); // every X seccond this will update the position of the obstakels
    unsigned long getDistanceTravelled() { return distance_travelled; };
private:
    void update_obstacles(unsigned long delta_time);
    void create_obstacle();
};


#endif //PEOPLEPARTY_BACKEND_HIGHWAY_HUSTLE_MAP_H