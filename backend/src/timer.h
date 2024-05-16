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
    std::atomic<bool> active{true};

public:
    void setTimeout(auto function, int delay);
    void setInterval(auto function, int interval);
    void stop();

};

void Timer::setTimeout(auto function, int delay) {
    active = true;
    std::thread t([&]() {
        if(!active.load()) return;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        if(!active.load()) return;
        function();
    });
    t.detach();
}

void Timer::setInterval(auto function, int interval) {
    active = true;
    std::thread t([&]() {
        while(active.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            if(!active.load()) return;
            function();
        }
    });
    t.detach();
}

inline void Timer::stop() {
    active = false;
}

#endif //PEOPLEPARTY_BACKEND_TIMER_H