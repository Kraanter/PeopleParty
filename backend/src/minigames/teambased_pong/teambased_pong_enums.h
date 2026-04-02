#ifndef PEOPLEPARTY_BACKEND_TEAMBASED_PONG_ENUMS_H
#define PEOPLEPARTY_BACKEND_TEAMBASED_PONG_ENUMS_H

enum class PongPhase : uint8_t {
    ROUND_PREP = 0,   // Showing team assignments before round
    PLAYING = 1,      // Active pong gameplay
    ROUND_RESULT = 2  // Showing results between rounds
};

enum class Team : uint8_t {
    TEAM_A = 0,
    TEAM_B = 1,
    SPECTATOR = 2
};

#endif //PEOPLEPARTY_BACKEND_TEAMBASED_PONG_ENUMS_H