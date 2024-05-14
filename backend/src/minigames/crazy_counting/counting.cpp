//
// Created by itssiem on 5/14/24.
//

#include "counting.h"

int CountingRegister::increase(int client_id) {
    if (counting_register.find(client_id) == counting_register.end()) {
        counting_register[client_id] = 0;
    }
    return ++counting_register[client_id];
}

int CountingRegister::decrease(int client_id) {
    if (counting_register.find(client_id) == counting_register.end()) {
        counting_register[client_id] = 0;
    }
    return --counting_register[client_id];
}

int CountingRegister::get_count(int client_id) {
    if (counting_register.find(client_id) == counting_register.end()) {
        counting_register[client_id] = 0;
    }
    return counting_register[client_id];
}