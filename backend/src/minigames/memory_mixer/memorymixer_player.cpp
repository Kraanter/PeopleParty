#include "memorymixer_player.h"

MemoryMixer_Player::MemoryMixer_Player() : MemoryMixer_Player(-1) {
}

MemoryMixer_Player::MemoryMixer_Player(int client_id) {
    this->client_id = client_id;
    submitted_x = -1;
    submitted_y = -1;
}

void MemoryMixer_Player::submit(int x, int y) {
    submitted_x = x;
    submitted_y = y;
    submitted_time = std::chrono::system_clock::now().time_since_epoch().count();
}

bool MemoryMixer_Player::operator<(const MemoryMixer_Player &other) const {
    if (submitted_time != other.submitted_time) {
        return submitted_time < other.submitted_time;
    }
    return rand() % 2 == 0;
}