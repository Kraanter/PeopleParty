#include "marble_mania_mini_game.h"

MarbleMania_MiniGame::MarbleMania_MiniGame(Game *game) : MiniGame(game) {
    // Initialize map with world bounds and finish line
    Vector2D worldMin(-400, -300);
    Vector2D worldMax(400, 400);
    float finishLine = 350.0f; // Y coordinate of finish line
    
    map = new MarbleManiaMap(worldMin, worldMax, finishLine);
    
    // Initialize game state
    placementPhaseComplete = false;
    placementTimeLeft = 15.0f; // 15 seconds to place marbles
}

MarbleMania_MiniGame::~MarbleMania_MiniGame() {
    introduction_timer.clear();
    minigame_timer.clear();
    result_timer.clear();
    timer.clear();
    
    delete map;
}

void MarbleMania_MiniGame::start_introduction() {
    update_interval = 500 MILLISECONDS;
    introduction_timer.setInterval([this]() { introduction_update(update_interval); }, update_interval);
}

void MarbleMania_MiniGame::pause() {
    minigame_timer.pause();
    result_timer.pause();
    introduction_timer.pause();
    timer.pause();
}

void MarbleMania_MiniGame::resume() {
    minigame_timer.resume();
    result_timer.resume();
    introduction_timer.resume();
    timer.resume();
}

void MarbleMania_MiniGame::introduction_update(int delta_time)
{
    introduction_time -= delta_time;

    if (introduction_time <= 0) {
        introduction_timer.clear();
        start_minigame();
        return;
    }

    send_minigame_introduction(get_camel_case_name(), introduction_time, get_display_name(), get_description());
}

void MarbleMania_MiniGame::start_minigame() {
    map->CreatePlayers(game->get_clients());

    update_interval = floor(1000 / target_fps);

    minigame_timer.setInterval([this]() { update(update_interval); }, update_interval);
}

void MarbleMania_MiniGame::process_input(const MiniGamePayloadType *payload, Client *from) {
    switch(payload->gamestatetype()) {
        case GameStateType_MarbleManiaPlayerInput: {
            auto input = payload->gamestatepayload_as_MarbleManiaPlayerInputPayload();
            
            // Handle marble placement during placement phase
            if (map->GetCurrentPhase() == GamePhase::PLACEMENT) {
                Vector2D position(input->x_pos(), input->y_pos());
                if (map->PlaceMarble(from, position)) {
                    // Send confirmation to player
                    send_player_update(from);
                }
            }
        }
    }
}

void MarbleMania_MiniGame::update(int delta_time) {
    float deltaTimeSeconds = delta_time / 1000.0f;
    
    // Update map
    map->Update(deltaTimeSeconds);
    
    // Handle placement phase
    if (map->GetCurrentPhase() == GamePhase::PLACEMENT) {
        placementTimeLeft -= deltaTimeSeconds;
        
        // Check if all players are ready or time is up
        if (map->AllPlayersReady() || placementTimeLeft <= 0) {
            map->StartPhysicsSimulation();
            placementPhaseComplete = true;
        }
    }
    
    // Check if game is finished
    if (map->GetCurrentPhase() == GamePhase::FINISHED) {
        minigame_timer.clear();
        start_result();
        return;
    }
    
    // Send updates to clients
    send_host_update();
    
    // Send individual updates to players
    for (const auto& pair : map->GetPlayerMarbles()) {
        send_player_update(pair.first);
    }
}

void MarbleMania_MiniGame::send_host_update()
{
    // Create the flatbuffer object
    flatbuffers::FlatBufferBuilder builder;
    
    // Create entity data for all marbles and obstacles
    std::vector<flatbuffers::Offset<FBMarbleManiaEntity>> entities;
    
    // Add player marbles
    for (const auto& pair : map->GetPlayerMarbles()) {
        const MarbleManiaMarble* marble = pair.second.get();
        Vector2D pos = marble->GetPosition();
        
        auto entityOffset = CreateFBMarbleManiaEntity(builder,
            builder.CreateString(marble->GetId()),
            pos.x, pos.y,
            0, // 0 = marble
            marble->HasFinished());
        entities.push_back(entityOffset);
    }
    
    // Add obstacles
    for (const auto& obstacle : map->GetObstacles()) {
        Vector2D pos = obstacle->GetPosition();
        
        auto entityOffset = CreateFBMarbleManiaEntity(builder,
            builder.CreateString(obstacle->GetId()),
            pos.x, pos.y,
            1, // 1 = obstacle
            false);
        entities.push_back(entityOffset);
    }
    
    // Create the host payload
    auto entitiesVector = builder.CreateVector(entities);
    auto hostPayload = CreateMarbleManiaHostPayload(builder, 
        entitiesVector,
        static_cast<uint8_t>(map->GetCurrentPhase()),
        placementTimeLeft,
        map->GetFinishLine());
    
    // Send payload to host
    game->party->send_gamestate([](Client* client) { return client->party->host == client; }, 
                               builder, hostPayload.Union());
}

void MarbleMania_MiniGame::send_player_update(Client *client) {
    flatbuffers::FlatBufferBuilder builder;
    
    // Create entity data for this player's marble
    std::vector<flatbuffers::Offset<FBMarbleManiaEntity>> entities;
    
    auto it = map->GetPlayerMarbles().find(client);
    if (it != map->GetPlayerMarbles().end()) {
        const MarbleManiaMarble* marble = it->second.get();
        Vector2D pos = marble->GetPosition();
        
        auto entityOffset = CreateFBMarbleManiaEntity(builder,
            builder.CreateString(marble->GetId()),
            pos.x, pos.y,
            0, // 0 = marble
            marble->HasFinished());
        entities.push_back(entityOffset);
    }
    
    // Create the host payload (reusing the same structure)
    auto entitiesVector = builder.CreateVector(entities);
    auto hostPayload = CreateMarbleManiaHostPayload(builder, 
        entitiesVector,
        static_cast<uint8_t>(map->GetCurrentPhase()),
        placementTimeLeft,
        map->GetFinishLine());
    
    // Send payload to specific client
    game->party->send_gamestate([client](Client* c) { return c == client; }, 
                               builder, hostPayload.Union());
}

void MarbleMania_MiniGame::start_result() {


    // send_result_data(game->party->host->client_id);
    // for (auto &player : map->players) {
    //     send_result_data(player.first->client_id);
    // }

    result_timer.setTimeout([this]() {
        finished();
    }, 5 SECONDS);
}

void MarbleMania_MiniGame::send_result_data(int client_id)
{
    flatbuffers::FlatBufferBuilder builder;
    auto mini_game_result = getMinigameResult();
    
    // Create result pairs
    std::vector<flatbuffers::Offset<FBMarbleManiaResultPair>> results;
    
    for (const auto& pair : mini_game_result) {
        // Find the marble for this client to get timing information
        auto marbleIt = map->GetPlayerMarbles().find(pair.first);
        float timeToFinish = 0.0f;
        bool hasFinished = false;
        
        if (marbleIt != map->GetPlayerMarbles().end()) {
            timeToFinish = marbleIt->second->GetTimeToFinish();
            hasFinished = marbleIt->second->HasFinished();
        }
        
        auto resultPair = CreateFBMarbleManiaResultPair(builder,
            builder.CreateString(pair.first->name),
            pair.second,
            timeToFinish,
            hasFinished);
        results.push_back(resultPair);
    }
    
    // Create the result payload
    auto resultsVector = builder.CreateVector(results);
    auto resultPayload = CreateMarbleManiaResultPayload(builder, resultsVector);
    
    // Create the game state payload
    // auto gameStatePayload = CreateGameStatePayload(builder, 
    //     GameStatePayloadType_MarbleManiaResultPayload, resultPayload.Union());
    
    // Send result to specific client
    game->party->send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, 
                               builder, resultPayload.Union());
}

std::vector<std::pair<Client *, int>> MarbleMania_MiniGame::getMinigameResult() {
    return map->GetFinalPlacements();
}