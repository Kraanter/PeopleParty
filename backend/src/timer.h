//
// Created by itssiem on 5/16/24.
//

#ifndef PEOPLEPARTY_BACKEND_TIMER_H
#define PEOPLEPARTY_BACKEND_TIMER_H

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

class Timer {
    bool active = false;

public:
    void startUpdateTimer(GameState* gameState);
    void stop();

};

inline void Timer::startUpdateTimer(GameState* gameState) {
    active = true;
        while(active) {
            std::this_thread::sleep_for(std::chrono::milliseconds(gameState->update_interval));
            gameState->update(gameState->update_interval);
        }
}

inline void Timer::stop() {
    active = false;
}

#endif //PEOPLEPARTY_BACKEND_TIMER_H
