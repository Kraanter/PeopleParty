//
// Created by itssiem on 6/7/24.
//

#include <cmath>
#include "rps_bracket_minigame.h"
#include "../../game.h"

RPSBracket_MiniGame::RPSBracket_MiniGame(Game *game) : MiniGame(game) {

}

void print_matches(std::vector<RPS_Match> matches) {
    for (RPS_Match &match : matches) {
        std::cout << "Player1: " << (match.player1 != nullptr ? match.player1->name : "null") << " Player2: " << (match.player2 != nullptr ? match.player2->name : "null") << " Winner: " << (match.winner != nullptr ? match.winner->name : "null") << std::endl;
    }
}

void RPSBracket_MiniGame::start_minigame() {
    for ( auto client : game->get_clients()) {
        if (client->isHost) continue;
        players.push_back(client);
    }
    create_matches(players);
    print_matches(matches);
    timer.setInterval([this]() { update(update_interval); }, update_interval);
}

void RPSBracket_MiniGame::create_matches(std::vector<Client *> players) {
    const int pn = players.size();
    const int rn = (int) ceil(log2(pn));
    const int mn = pow(2, rn) - 1;
    matches.resize(mn);

    for (int i = 0; i < mn; i++) {
        matches[i].player1 = nullptr;
        matches[i].player2 = nullptr;
        matches[i].player1_choice = RPS_Choice::NONE;
        matches[i].player2_choice = RPS_Choice::NONE;
        matches[i].winner = nullptr;
        matches[i].remaining_time = match_time;
    }

    int matches_in_outer_round = mn / 2 + 1;
    int start_index = matches_in_outer_round - 1;
    for (int i = 0; i < start_index; i++) {
        matches[start_index + i].player1 = players[i];
        if (matches_in_outer_round + i < pn) {
            matches[start_index + i].player2 = players[i + start_index];
        }
    }
}

void RPSBracket_MiniGame::evaluate_match(RPS_Match* match) {

    // If one player is missing, the other player wins
    if (match->player1 == nullptr || match->player2 == nullptr) {
        if (match->player1 == nullptr) {
            match->winner = match->player2;
        } else {
            match->winner = match->player1;
        }
        promote_winners();
        return;
    }

    // If both players have not made a choice, randomly select a winner
    if (match->player1_choice == NONE && match->player2_choice == NONE) {
        if (rand() % 2 == 0) {
            match->winner = match->player1;
        } else {
            match->winner = match->player2;
        }
        std::cout << match->winner->name << " won" << std::endl;
        promote_winners();
        return;
    }

    // If one player has not made a choice, the other player wins
    if (match->player1_choice == NONE || match->player2_choice == NONE) {
        if (match->player1_choice == NONE) {
            match->winner = match->player2;
        } else {
            match->winner = match->player1;
        }
        std::cout << match->winner->name << " won" << std::endl;
        promote_winners();
        return;
    }

    // If both players have made a choice, evaluate the match
    if (match->player1_choice == match->player2_choice) {
        match->remaining_time = match_time;
        match->player1_choice = NONE;
        match->player2_choice = NONE;
    } else if (match->player1_choice == ROCK && match->player2_choice == SCISSORS) {
        match->winner = match->player1;
    } else if (match->player1_choice == PAPER && match->player2_choice == ROCK) {
        match->winner = match->player1;
    } else if (match->player1_choice == SCISSORS && match->player2_choice == PAPER) {
        match->winner = match->player1;
    } else {
        match->winner = match->player2;
    }

    std::cout << match->winner->name << " won" << std::endl;
    promote_winners();
}

void RPSBracket_MiniGame::promote_winners() {
    for (int i = 0; i < matches.size(); i++) {
        RPS_Match& match = matches[i];
        int ri = log2(i + 1);
        int rl = pow(2, ri) - 1;
        int li = i - rl;
        int p1i = li * 2 + (pow(2, (ri + 1)) - 1);
        int p2i = p1i + 1;
        std::cout << "i: " << i << " p1i: " << p1i << " p2i: " << p2i << std::endl;
        if (p1i >= matches.size() || p2i >= matches.size()) {
            continue;
        }
        match.player1 = matches[p1i].winner;
        match.player2 = matches[p2i].winner;
    }
}

void RPSBracket_MiniGame::update_matches(int delta_time) {
    for (RPS_Match &match : matches) {
        if (match.winner != nullptr) {
            continue;
        }

        if (match.player1 == nullptr || match.player2 == nullptr) {
            evaluate_match(&match);
            continue;
        }

        if (match.remaining_time <= 0 || (match.player1_choice != RPS_Choice::NONE && match.player2_choice != RPS_Choice::NONE)) {
            evaluate_match(&match);
        }

        match.remaining_time -= delta_time;
    }
}

void RPSBracket_MiniGame::update(int delta_time) {
    if (matches[0].winner != nullptr) {
        std::cout << "winner is: " << matches[0].winner->name << std::endl;
        start_result();
    }
    update_matches(delta_time);
    send_host_update();
    send_players_update();
}

void RPSBracket_MiniGame::send_host_update() {
    flatbuffers::FlatBufferBuilder builder;
    std::vector<flatbuffers::Offset<FB_RPSMatch>> matchesvec;
    for (RPS_Match &match : matches) {
        auto name1 =  builder.CreateString((match.player1 != nullptr) ? match.player1->name.c_str() : "");
        auto name2 =  builder.CreateString((match.player2 != nullptr) ? match.player2->name.c_str() : "");
        auto winner =  builder.CreateString((match.winner != nullptr) ? match.winner->name.c_str() : "");
        auto matchbuf = CreateFB_RPSMatch(builder, name1, name2, winner);
        matchesvec.push_back(matchbuf);
    }
    auto matchesbuf = builder.CreateVector(matchesvec);
    auto payload = CreateRPSBracketHostPayload(builder, matchesbuf);
    auto gamestate = CreateMiniGamePayloadType(builder, builder.CreateString(get_camel_case_name()), GameStateType_RPSBracketHost, GameStatePayload_RPSBracketHostPayload, payload.Union());
    game->party->send_gamestate([](Client* client) { return client->party->host == client; }, builder, gamestate.Union());
}

void RPSBracket_MiniGame::start_introduction() {
    start_minigame();
}

void RPSBracket_MiniGame::start_result() {
    timer.setTimeout([this]() {
        finished();
    }, 10 SECONDS);
}

std::vector<Client *> RPSBracket_MiniGame::getMinigameResult() {
    return std::vector<Client *>();
}

void RPSBracket_MiniGame::process_input(const MiniGamePayloadType *payload, Client *from) {

}

void RPSBracket_MiniGame::send_players_update() {

}

