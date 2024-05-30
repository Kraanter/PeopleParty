#ifndef Party_Prep_H
#define Party_Prep_H

#include "../game_state.h"

class Game;

class PartyPrep: public GameState {
public:
    PartyPrep(Game *game);
    void clients_changed(int client_id, bool joined) override;
protected:
    void finished() override;
private:
    void send_host_information();
    void send_player_information(int client_id);
    void process_input(const Message* payload, Client* from) override;
    void update(int delta_time) override;
};

#endif  // Party_Prep_H