//
// Created by itssiem on 5/16/24.
//

#include "crazycounting_player.h"

CrazyCounting_Player::CrazyCounting_Player() : CrazyCounting_Player(-1){
}

CrazyCounting_Player::CrazyCounting_Player(int client_id) {
    this->client_id = client_id;
}

int CrazyCounting_Player::increment_count() {
    if (submitted) return count;
    count++;
    last_changed = std::chrono::system_clock::now().time_since_epoch().count();
    return count;
}

int CrazyCounting_Player::decrement_count() {
    if (submitted) return count;
    count--;
    last_changed = std::chrono::system_clock::now().time_since_epoch().count();
    return count;
}

int CrazyCounting_Player::get_count() {
    return count;
}

void CrazyCounting_Player::submit() {
    submitted = true;
    last_changed = std::chrono::system_clock::now().time_since_epoch().count();
}
