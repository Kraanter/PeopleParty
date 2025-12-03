#include "teambased_pong_player.h"

TeambasedPong_Player::TeambasedPong_Player() 
    : client(nullptr), team(Team::SPECTATOR), played_matches(0), joystick_y(0.0f) {
}

TeambasedPong_Player::TeambasedPong_Player(Client* client, Team team)
    : client(client), team(team), played_matches(0), joystick_y(0.0f) {
}
