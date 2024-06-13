//
// Created by itssiem on 6/7/24.
//

#include <cmath>
#include "rps_bracket_minigame.h"
#include "../../game.h"

RPSBracket_MiniGame::RPSBracket_MiniGame(Game *game) : MiniGame(game) {

}

RPSBracket_MiniGame::~RPSBracket_MiniGame() {
    timer.clear();
    result_timer.clear();
}

void RPSBracket_MiniGame::start_minigame() {
    for ( auto client : game->get_clients()) {
        if (client->isHost) continue;
        players.push_back(client);
    }
    if (players.size() < 2) {
        finished();
        return;
    }
    create_matches(players);
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
    std::vector<int> population_order;
    for(int i = 0; i < matches_in_outer_round / 2; i++) {
        population_order.push_back(start_index + i);
        population_order.push_back(start_index + i + matches_in_outer_round / 2);
    }
    for(int i = 0; i < population_order.size(); i++) {
        int index = population_order[i];
        matches[index].player1 = players[i];
    }

    for(int i = 0; i < population_order.size(); i++) {
        int index = population_order[i];
        if (i + population_order.size() >= players.size()) {
            break;
        }
        matches[index].player2 = players[i + population_order.size()];
    }

    for(int i = 0; i < matches.size(); i++) {
        if (matches[i].player1 == nullptr || matches[i].player2 == nullptr) {
            evaluate_match(&matches[i]);
            matches[i].player1 = nullptr;
            matches[i].player2 = nullptr;
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
            minigame_result.push(match->player2);
        } else {
            match->winner = match->player2;
            minigame_result.push(match->player1);
        }
        promote_winners();
        return;
    }

    // If one player has not made a choice, the other player wins
    if (match->player1_choice == NONE || match->player2_choice == NONE) {
        if (match->player1_choice == NONE) {
            match->winner = match->player2;
            minigame_result.push(match->player1);
        } else {
            match->winner = match->player1;
            minigame_result.push(match->player2);
        }
        promote_winners();
        return;
    }

    // If both players have made a choice, evaluate the match
    if (match->player1_choice == match->player2_choice) {
        match->remaining_time = match_time;
        match->player1_choice = NONE;
        match->player2_choice = NONE;
        return;
    } else if (match->player1_choice == ROCK && match->player2_choice == SCISSORS) {
        match->winner = match->player1;
        minigame_result.push(match->player2);
    } else if (match->player1_choice == PAPER && match->player2_choice == ROCK) {
        match->winner = match->player1;
        minigame_result.push(match->player2);
    } else if (match->player1_choice == SCISSORS && match->player2_choice == PAPER) {
        match->winner = match->player1;
        minigame_result.push(match->player2);
    } else {
        match->winner = match->player2;
        minigame_result.push(match->player1);
    }

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
        if (p1i >= matches.size() || p2i >= matches.size()) {
            continue;
        }
        match.player1 = matches[p1i].winner;
        match.player2 = matches[p2i].winner;

        // send match result to show who won or lost
        if (matches[p1i].winner != nullptr) {
            send_match_update(matches[p1i]);
        }
        if (matches[p2i].winner != nullptr) {
            send_match_update(matches[p2i]);
        }
    }
}

void RPSBracket_MiniGame::update_matches(int delta_time) {
    for (RPS_Match &match : matches) {
        if (match.winner != nullptr) {
            continue;
        }

        if (match.player1 == nullptr || match.player2 == nullptr) {
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
        minigame_result.push(matches[0].winner);
        start_result();
        timer.clear();
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
    const int introduction_interval = 500 MILLISECONDS;
    introduction_timer.setInterval([this]() {
        introduction_update(introduction_interval);
    }, introduction_interval);
}

void RPSBracket_MiniGame::introduction_update(int dt) {
    introduction_time -= dt;
    if (introduction_time <= 0) {
        introduction_timer.clear();
        start_minigame();
        return;
    }
    send_minigame_introduction(get_camel_case_name(), introduction_time, get_display_name(), get_description());
}

void RPSBracket_MiniGame::start_result() {
    result_timer.setTimeout([this]() {
        finished();
    }, 5 SECONDS);
}

std::vector<Client *> RPSBracket_MiniGame::getMinigameResult() {
    std::vector<Client *> res;
    while (!minigame_result.empty()) {
        res.push_back(minigame_result.top());
        minigame_result.pop();
    }
    return res;
}

void RPSBracket_MiniGame::process_input(const MiniGamePayloadType *payload, Client *from) {
    switch (payload->gamestatetype()) {
        case GameStateType_RPSBracketPlayerInput: {
            auto input = payload->gamestatepayload_as_RPSBracketPlayerInputPayload();
            for (RPS_Match &match : matches) {
                if (match.winner == nullptr) {
                    if (match.player1 == from) {
                        match.player1_choice = (RPS_Choice) input->choice();
                        send_match_update(match);
                    } else if (match.player2 == from) {
                        match.player2_choice = (RPS_Choice) input->choice();
                        send_match_update(match);
                    }
                }
            }
            break;
        }
    }
}

void RPSBracket_MiniGame::send_players_update() {
    for (auto match : matches) {
        // also only send when match remaining time is under 10, gives time to show the previous match result
        if (match.remaining_time > 0 && match.remaining_time < 10 SECONDS && (match.player1 != nullptr || match.player2 != nullptr)) {
            send_match_update(match);
        }
    }
}

void RPSBracket_MiniGame::send_match_update(const RPS_Match match) {
    if (match.player1 != nullptr) {
        flatbuffers::FlatBufferBuilder builder;
        auto winner = builder.CreateString(match.winner != nullptr ? match.winner->name : "");
        auto opponent_name = match.player2 != nullptr ? match.player2->name : "";
        auto payload = CreateRPSBracketPlayerPayload(builder, (FB_RPSChoice) match.player1_choice, (FB_RPSChoice) match.player2_choice, builder.CreateString(opponent_name), winner, match.remaining_time);
        auto gamestatePayload = CreateMiniGamePayloadType(builder, builder.CreateString(get_camel_case_name()), GameStateType_RPSBracketPlayer, GameStatePayload_RPSBracketPlayerPayload, payload.Union());
        this->game->party->send_gamestate([match](Client* c){ return c->client_id == match.player1->client_id;}, builder, gamestatePayload.Union());
    }
    if (match.player2 != nullptr) {
        flatbuffers::FlatBufferBuilder builder;
        auto winner = builder.CreateString(match.winner != nullptr ? match.winner->name : "");
        auto opponent_name = match.player1 != nullptr ? match.player1->name : "";
        auto payload = CreateRPSBracketPlayerPayload(builder, (FB_RPSChoice) match.player2_choice, (FB_RPSChoice) match.player1_choice, builder.CreateString(opponent_name), winner, match.remaining_time);
        auto gamestatePayload = CreateMiniGamePayloadType(builder, builder.CreateString(get_camel_case_name()), GameStateType_RPSBracketPlayer, GameStatePayload_RPSBracketPlayerPayload, payload.Union());
        this->game->party->send_gamestate([match](Client* c){ return c->client_id == match.player2->client_id;}, builder, gamestatePayload.Union());
    }
}

