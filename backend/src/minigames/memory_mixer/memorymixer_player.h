#ifndef MEMORYMIXER_PLAYER_H
#define MEMORYMIXER_PLAYER_H


#include <chrono>

class MemoryMixer_Player {
public:
    MemoryMixer_Player();
    MemoryMixer_Player(int client_id);
public:
    int client_id;
    int submitted_x;
    int submitted_y;
    unsigned long submitted_time = std::chrono::system_clock::now().time_since_epoch().count();
private:
public:
    void submit(int x, int y);
};

#endif //MEMORYMIXER_PLAYER_H