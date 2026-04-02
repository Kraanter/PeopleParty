#ifndef PEOPLEPARTY_BACKEND_TEAMBASED_PONG_PLAYER_H
#define PEOPLEPARTY_BACKEND_TEAMBASED_PONG_PLAYER_H

#include "../../client.h"
#include "teambased_pong_enums.h"

class TeambasedPong_Player {
public:
    TeambasedPong_Player();
    TeambasedPong_Player(Client* client, Team team);
    
    Client* client;
    Team team;
    int played_matches = 0;
    float joystick_y = 0.0f; // -1 to 1, input from joystick
    
    bool operator<(const TeambasedPong_Player& other) const {
        return played_matches > other.played_matches; // More matches = higher ranking
    }
};

#endif //PEOPLEPARTY_BACKEND_TEAMBASED_PONG_PLAYER_H
