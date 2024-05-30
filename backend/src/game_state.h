//
// Created by itssiem on 5/15/24.
//

#ifndef PEOPLEPARTY_BACKEND_GAME_STATE_H
#define PEOPLEPARTY_BACKEND_GAME_STATE_H


#include "flatbuffer/messageClass_generated.h"

class Game;
class Client;

class GameState {
public:
    GameState(Game* game);
    virtual void process_input(const Message* payload, Client* from) = 0;
    virtual void update(int delta_time) = 0;
    int update_interval;
    virtual void clients_changed(int client_id, bool joined) = 0;
protected:
    virtual void finished() = 0;
    Game* game;
};


#endif //PEOPLEPARTY_BACKEND_GAME_STATE_H
