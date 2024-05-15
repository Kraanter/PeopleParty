//
// Created by itssiem on 5/14/24.
//

#ifndef PEOPLEPARTY_BACKEND_CRAZYCOUNTING_COUNTING_REGISTER_H
#define PEOPLEPARTY_BACKEND_CRAZYCOUNTING_COUNTING_REGISTER_H

#include <map>

class CrazyCounting_CountingRegister {
public:
    CrazyCounting_CountingRegister() = default;
    int increase(int client_id);
    int decrease(int client_id);
    int get_count(int client_id);
private:
    std::map<int, int> counting_register;
};

#endif //PEOPLEPARTY_BACKEND_CRAZYCOUNTING_COUNTING_REGISTER_H
