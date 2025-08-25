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
            
            // Handle marble lock-in during placement phase
            if (map->GetCurrentPhase() == GamePhase::PLACEMENT) {
                if (input->lock()) {
                    map->SetPlayerReady(from, true);
                }
            }
            break;
        }
        case GameStateType_JoystickData: {
            auto input = payload->gamestatepayload_as_JoystickDataPayload();
            
            // Handle marble movement during placement phase
            if (map->GetCurrentPhase() == GamePhase::PLACEMENT) {
                float x = input->x_pos();
                float y = -input->y_pos(); // invert Y axis for screen coordinates
                
                // Move the player's marble in the drop zone
                map->MovePlayerMarble(from, x, y);
            }
            break;
        }
        default:
            break;
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
    flatbuffers::FlatBufferBuilder builder;

    std::vector<flatbuffers::Offset<FBMarbleManiaEntity>> entities;

    // Helper: create FBVec2
    auto make_vec2 = [&](float x, float y) {
        return CreateFBVec2(builder, x, y);
    };

    // --- Player marbles ---
    for (const auto& pair : map->GetPlayerMarbles()) {
        const Client* client = pair.first;
        const MarbleManiaMarble* marble = pair.second.get();
        Vector2D p = marble->GetPosition();

        // Shape = circle
        auto circle = CreateFBCircle(builder, marble->GetRadius());
        auto pos    = make_vec2(p.x, p.y);
        auto id     = builder.CreateString(marble->GetId());
        auto name   = builder.CreateString(client->name);

        auto entity = CreateFBMarbleManiaEntity(
            builder,
            id,
            FBEntityType::FBEntityType_Marble,
            pos,
            /*rotation=*/0.0f,                 // marbles visually don't need rotation
            /*finished=*/marble->HasFinished(),
            name,
            FBShape::FBShape_FBCircle,         // union type
            circle.Union(),                    // union value
            /*restitution=*/0.8f,
            /*friction=*/0.2f
        );
        entities.push_back(entity);
    }

    // --- Obstacles (static) ---
    for (const auto& obsPtr : map->GetObstacles()) {
        const auto& obs = *obsPtr;
        Vector2D p = obs.GetPosition();

        flatbuffers::Offset<void> shape_value;
        FBShape shape_type = FBShape::FBShape_NONE;

        switch (obs.GetObstacleType()) {
            case ObstacleType::Circle: {
                float radius = obs.GetWidth() * 0.5f;
                auto circle = CreateFBCircle(builder, radius);
                shape_type  = FBShape::FBShape_FBCircle;
                shape_value = circle.Union();
                break;
            }
            case ObstacleType::Rectangle: {
                auto rect = CreateFBRect(builder, obs.GetWidth(), obs.GetHeight());
                shape_type  = FBShape::FBShape_FBRect;
                shape_value = rect.Union();
                break;
            }
            case ObstacleType::Triangle: {
                // Provide polygon vertices in LOCAL space (pre-rotation)
                auto vertsLocal = obs.GetTriangleLocalVerts();
                std::vector<flatbuffers::Offset<FBVec2>> vertsFB;
                vertsFB.reserve(vertsLocal.size());
                for (auto& v : vertsLocal) {
                    vertsFB.push_back(CreateFBVec2(builder, v.x, v.y));
                }
                auto vertsVec = builder.CreateVector(vertsFB);
                auto poly = CreateFBPoly(builder, vertsVec);
                shape_type  = FBShape::FBShape_FBPoly;
                shape_value = poly.Union();
                break;
            }
        }

        auto pos  = make_vec2(p.x, p.y);
        auto id   = builder.CreateString(obs.GetId());
        auto name = builder.CreateString("");   // obstacles don't have names

        auto entity = CreateFBMarbleManiaEntity(
            builder,
            id,
            FBEntityType::FBEntityType_Obstacle,
            pos,
            /*rotation=*/obs.GetCurrentRotation(),
            /*finished=*/false,
            name,
            shape_type,
            shape_value,
            /*restitution=*/0.6f,
            /*friction=*/0.4f
        );
        entities.push_back(entity);
    }

    // --- Host payload ---
    auto entitiesVector = builder.CreateVector(entities);
    auto worldMin = map->GetWorldMin();   // add simple getters if you don't have them yet
    auto worldMax = map->GetWorldMax();

    auto payload = CreateMarbleManiaHostPayload(
        builder,
        entitiesVector,
        static_cast<uint8_t>(map->GetCurrentPhase()),
        placementTimeLeft,
        map->GetFinishLine(),
        CreateFBVec2(builder, worldMin.x, worldMin.y),
        CreateFBVec2(builder, worldMax.x, worldMax.y)
    );

    auto miniGame = builder.CreateString(get_camel_case_name());
    auto gameStatePayload = CreateMiniGamePayloadType(
        builder, miniGame,
        GameStateType_MarbleManiaHost,
        GameStatePayload_MarbleManiaHostPayload,
        payload.Union()
    );

    game->party->send_gamestate(
        [](Client* client) { return client->party->host == client; },
        builder, gameStatePayload.Union()
    );
}


void MarbleMania_MiniGame::send_player_update(Client *client) {
    flatbuffers::FlatBufferBuilder builder;

    // find the entity for this client
    auto marbleIt = map->GetPlayerMarbles().find(client);
    if (marbleIt == map->GetPlayerMarbles().end()) {
        // Client does not have a marble, skip sending update
        return;
    }

    auto hostPayload = CreateMarbleManiaPlayerPayload(builder, 
        builder.CreateString(marbleIt->second->GetId()),
        static_cast<uint8_t>(map->GetCurrentPhase()),
        placementTimeLeft,
        marbleIt->second->GetPosition().y,
        map->GetFinishLine());

    auto miniGame = builder.CreateString(get_camel_case_name());
    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_MarbleManiaPlayer,
                                                      GameStatePayload_MarbleManiaPlayerPayload, hostPayload.Union());
    
    // Send payload to specific client
    game->party->send_gamestate([client](Client* c) { return c == client; }, 
                               builder, gameStatePayload.Union());
}

void MarbleMania_MiniGame::start_result() {
    send_result_data(game->party->host->client_id);
    for (const auto& pair : map->GetPlayerMarbles()) {
        send_result_data(pair.first->client_id);
    }

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

    auto miniGame = builder.CreateString(get_camel_case_name());
    auto gameStatePayload = CreateMiniGamePayloadType(builder, miniGame, GameStateType_MarbleManiaResult,
                                                      GameStatePayload_MarbleManiaResultPayload, resultPayload.Union());
    
    // Send result to specific client
    game->party->send_gamestate([client_id](Client* client) { return client->client_id == client_id; }, builder, gameStatePayload.Union());
}

std::vector<std::pair<Client *, int>> MarbleMania_MiniGame::getMinigameResult() {
    return map->GetFinalPlacements();
}