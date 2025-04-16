#ifndef PEOPLEPARTY_BACKEND_HIGHWAY_HUSTLE_MAP_H
#define PEOPLEPARTY_BACKEND_HIGHWAY_HUSTLE_MAP_H

#include <map>
#include <vector>
#include "moving_entity.h"
#include "../../client.h"

class Client;

class HighwayHustle_Map {
private:
    int map_height;
    int map_width;
public:
    // list of players (players have a x and y position and a function to move it (takes a vector))
    std::map<Client*, Moving_Entity*> players;
    // tbd: list of entities (obstacles)
public:
    HighwayHustle_Map() { HighwayHustle_Map(0, 0); };
    HighwayHustle_Map(float x, float y);
    ~HighwayHustle_Map();
    void update(unsigned long delta_time);
    void create_players(std::vector<Client *> players);
    void update_player_velocity(Client* player, float x, float y);
    void update_player_event(Client* player, int event_type); // can be stop (0) or start (1), joystick events
    //void update_obstacle(); // every X seccond this will update the position of the obstakels
};


#endif //PEOPLEPARTY_BACKEND_HIGHWAY_HUSTLE_MAP_H