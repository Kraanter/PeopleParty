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
    std::atomic<bool> active;
    std::thread timerThread;

public:
    Timer() : active(false) {}

    void startUpdateTimer(std::function<void(int)> callback, int interval);
    void stop();
    ~Timer() {
        stop();
    }
    void add_timeout(std::function<void()> callback, int timeout) {
        std::thread([callback, timeout]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
            callback();
        }).detach();
    }
};

inline void Timer::startUpdateTimer(std::function<void(int)> callback, int interval) {
    active = true;
    timerThread = std::thread([this, callback, interval]() {
        while (active) {
            auto start_time = std::chrono::steady_clock::now();
            
            // Call update
            callback(interval);

            // Calculate how much time to sleep to maintain the interval
            auto elapsed = std::chrono::steady_clock::now() - start_time;
            auto sleep_duration = std::chrono::milliseconds(interval) - elapsed;
            
            if (sleep_duration > std::chrono::milliseconds(0)) {
                std::this_thread::sleep_for(sleep_duration);
            }
        }
    });
}

inline void Timer::stop() {
    active = false;
    if (timerThread.joinable()) {
        timerThread.detach();
    }
}

#endif //PEOPLEPARTY_BACKEND_TIMER_H
