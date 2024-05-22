//
// Created by itssiem on 5/16/24.
//

#include <cstdlib>
#include "crazycounting_player.h"

CrazyCounting_Player::CrazyCounting_Player() : CrazyCounting_Player(-1, -1){
}

CrazyCounting_Player::CrazyCounting_Player(int client_id) {
    this->client_id = client_id;
    submitted = false;
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

bool CrazyCounting_Player::operator<(const CrazyCounting_Player &other) const {
    if (abs(count - target) != abs(other.count - other.target)) {
        return abs(count - target) < abs(other.count - other.target);
    }
    if (submitted != other.submitted) {
        return submitted;
    }
    return last_changed < other.last_changed;
}
