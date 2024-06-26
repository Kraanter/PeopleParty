//
// Created by itssiem on 5/16/24.
//

#ifndef PEOPLEPARTY_BACKEND_CRAZYCOUNTING_PLAYER_H
#define PEOPLEPARTY_BACKEND_CRAZYCOUNTING_PLAYER_H

#include <chrono>

class CrazyCounting_Player {
public:
    CrazyCounting_Player();
    CrazyCounting_Player(int client_id, int target);
public:
    int client_id;
    bool submitted = false;
    unsigned long last_changed = std::chrono::system_clock::now().time_since_epoch().count();
private:
    int count = 0;
    int target;
public:
    int increment_count();
    int decrement_count();
    int get_count();
    void submit();
    bool operator<(const CrazyCounting_Player& other) const;
};

template <class T>
struct less {
    constexpr bool operator()(const T& lhs, const T& rhs) const {
        return lhs < rhs;
    }
};


#endif //PEOPLEPARTY_BACKEND_CRAZYCOUNTING_PLAYER_H
