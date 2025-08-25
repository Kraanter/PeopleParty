#include "marble_mania_map.h"
#include <algorithm>
#include <sstream>
#include <cmath>

MarbleManiaMap::MarbleManiaMap(const Vector2D& worldMin, const Vector2D& worldMax, float finishLine)
    : m_worldMin(worldMin), m_worldMax(worldMax), m_finishLine(finishLine)
    , m_currentPhase(GamePhase::PLACEMENT), m_gameTime(0.0f)
    , m_finishedMarbles(0), m_nextPlacement(1)
{
    // Create Box2D world with gravity
    b2Vec2 gravity(0.0f, 9.81f); // 9.81 m/s^2 downward
    m_world = std::make_unique<b2World>(gravity);
    
    // Set up drop zone (top portion of the world)
    float dropZoneHeight = 100.0f;
    m_dropZoneMin = Vector2D(worldMin.x + 50, worldMin.y + 20);
    m_dropZoneMax = Vector2D(worldMax.x - 50, worldMin.y + dropZoneHeight);
    
    // Create boundaries and obstacles
    CreateBoundaries();
    CreateObstacles();
}

MarbleManiaMap::~MarbleManiaMap() {
    // Clean up boundary bodies
    for (b2Body* body : m_boundaryBodies) {
        m_world->DestroyBody(body);
    }
    m_boundaryBodies.clear();
    
    // Box2D world will automatically clean up all remaining bodies
}

void MarbleManiaMap::CreatePlayers(const std::vector<Client*>& clients) {
    float marbleRadius = 0.15f; // 15cm in meters (Box2D uses meters)
    float spacing = 0.5f; // 50cm spacing
    float startX = (m_dropZoneMin.x + spacing) / 100.0f; // Convert to meters
    
    for (size_t i = 0; i < clients.size(); i++) {
        Client* client = clients[i];

        // If client is host, skip creating a marble
        if (client->isHost) {
            continue;
        }
        
        // Calculate initial position in drop zone (convert to meters)
        Vector2D position(
            startX + (i * spacing), 
            (m_dropZoneMin.y + 30) / 100.0f
        );
        
        // Ensure position is within drop zone
        if (position.x > (m_dropZoneMax.x - marbleRadius * 100.0f) / 100.0f) {
            position.x = (m_dropZoneMax.x - marbleRadius * 100.0f) / 100.0f;
        }
        
        // Create Box2D body for marble (initially static)
        b2Body* marbleBody = CreateCircleBody(position, marbleRadius, true);
        
        // Set physics properties
        b2Fixture* fixture = marbleBody->GetFixtureList();
        fixture->SetRestitution(0.8f);  // Bouncy
        fixture->SetFriction(0.2f);     // Low friction
        fixture->SetDensity(1.0f);      // Standard density
        marbleBody->ResetMassData();
        
        // Create marble wrapper
        auto marble = std::make_unique<MarbleManiaMarble>(marbleBody, client, marbleRadius);
        
        m_playerMarbles[client] = std::move(marble);
        m_playersReady[client] = false;
    }
}

void MarbleManiaMap::RemovePlayer(Client* client) {
    auto marbleIt = m_playerMarbles.find(client);
    if (marbleIt != m_playerMarbles.end()) {
        // Remove Box2D body from world
        m_world->DestroyBody(marbleIt->second->GetBody());
        
        // Remove marble and ready state
        m_playerMarbles.erase(marbleIt);
        m_playersReady.erase(client);
    }
}

bool MarbleManiaMap::PlaceMarble(Client* client, const Vector2D& position) {
    auto marbleIt = m_playerMarbles.find(client);
    if (marbleIt == m_playerMarbles.end()) {
        return false;
    }
    
    // Check if position is valid
    if (!IsValidDropPosition(position)) {
        return false;
    }
    
    // Check if position is occupied
    float marbleRadius = marbleIt->second->GetRadius();
    if (IsPositionOccupied(position, marbleRadius, marbleIt->second.get())) {
        return false;
    }
    
    // Place the marble
    marbleIt->second->SetPosition(position);
    marbleIt->second->SetPlaced(true);
    
    return true;
}

void MarbleManiaMap::MovePlayerMarble(Client* client, float deltaX, float deltaY) {
    if (m_currentPhase != GamePhase::PLACEMENT) {
        return;
    }
    
    auto marbleIt = m_playerMarbles.find(client);
    if (marbleIt == m_playerMarbles.end()) {
        return;
    }
    
    MarbleManiaMarble* marble = marbleIt->second.get();
    Vector2D currentPos = marble->GetPosition();
    Vector2D newPos = currentPos + Vector2D(deltaX * 5.0f, deltaY * 5.0f); // Scale movement
    
    // Clamp to drop zone
    float radius = marble->GetRadius();
    newPos.x = std::max(m_dropZoneMin.x + radius, std::min(m_dropZoneMax.x - radius, newPos.x));
    newPos.y = std::max(m_dropZoneMin.y + radius, std::min(m_dropZoneMax.y - radius, newPos.y));
    
    // Check if new position is occupied
    if (!IsPositionOccupied(newPos, radius, marble)) {
        marble->SetPosition(newPos);
    }
}

void MarbleManiaMap::SetPlayerReady(Client* client, bool ready) {
    auto it = m_playersReady.find(client);
    if (it != m_playersReady.end()) {
        it->second = ready;
    }
}

bool MarbleManiaMap::IsPlayerReady(Client* client) const {
    auto it = m_playersReady.find(client);
    return (it != m_playersReady.end()) ? it->second : false;
}

bool MarbleManiaMap::AllPlayersReady() const {
    for (const auto& pair : m_playersReady) {
        if (!pair.second) {
            return false;
        }
    }
    return !m_playersReady.empty();
}

void MarbleManiaMap::Update(float deltaTime) {
    m_gameTime += deltaTime;
    
    if (m_currentPhase == GamePhase::PHYSICS_SIMULATION) {
        UpdatePhysics(deltaTime);
        UpdateObstacles(deltaTime);
        CheckFinishLine();
        UpdatePlacements();
        
        // Check if all marbles have finished or are stuck
        if (m_finishedMarbles >= m_playerMarbles.size()) {
            m_currentPhase = GamePhase::FINISHED;
        }
    }
}

void MarbleManiaMap::StartPhysicsSimulation() {
    m_currentPhase = GamePhase::PHYSICS_SIMULATION;
    
    // Make all marbles dynamic so they fall
    for (auto& pair : m_playerMarbles) {
        pair.second->GetBody()->SetType(b2_dynamicBody);
    }
    
    m_gameTime = 0.0f; // Reset game time for finish time tracking
}

void MarbleManiaMap::UpdatePhysics(float deltaTime) {
    // Step the Box2D world
    int32 velocityIterations = 6;
    int32 positionIterations = 2;
    m_world->Step(deltaTime, velocityIterations, positionIterations);
}

void MarbleManiaMap::UpdateObstacles(float deltaTime) {
    for (auto& obstacle : m_obstacles) {
        obstacle->Update(deltaTime);
    }
}

void MarbleManiaMap::CheckFinishLine() {
    for (auto& pair : m_playerMarbles) {
        MarbleManiaMarble* marble = pair.second.get();
        
        if (!marble->HasFinished() && marble->GetPosition().y >= m_finishLine) {
            marble->SetFinished(true, m_gameTime);
            marble->SetPlacement(m_nextPlacement++);
            m_finishedMarbles++;
        }
    }
}

void MarbleManiaMap::UpdatePlacements() {
    // This method can be used for additional placement logic if needed
    // For now, placements are handled in CheckFinishLine()
}

void MarbleManiaMap::CreateObstacles() {
    CreateDefaultObstacles();
}

MarbleManiaObstacle* MarbleManiaMap::AddStaticCircle(const Vector2D& position, float radius) {
    // Convert to meters
    Vector2D pos(position.x / 100.0f, position.y / 100.0f);
    float r = radius / 100.0f;
    
    b2Body* body = CreateCircleBody(pos, r, true);
    
    // Set bouncy properties
    b2Fixture* fixture = body->GetFixtureList();
    fixture->SetRestitution(0.8f);
    
    std::string id = GenerateObstacleId("static_circle", position);
    auto obstacle = std::make_unique<MarbleManiaObstacle>(body, ObstacleType::STATIC_CIRCLE, id);
    
    MarbleManiaObstacle* obstaclePtr = obstacle.get();
    m_obstacles.push_back(std::move(obstacle));
    
    return obstaclePtr;
}

MarbleManiaObstacle* MarbleManiaMap::AddStaticRectangle(const Vector2D& position, float width, float height, float rotation) {
    // Convert to meters
    Vector2D pos(position.x / 100.0f, position.y / 100.0f);
    float w = width / 100.0f;
    float h = height / 100.0f;
    
    b2Body* body = CreateRectangleBody(pos, w, h, rotation, true);
    
    // Set bouncy properties
    b2Fixture* fixture = body->GetFixtureList();
    fixture->SetRestitution(0.8f);
    
    std::string id = GenerateObstacleId("static_rect", position);
    auto obstacle = std::make_unique<MarbleManiaObstacle>(body, ObstacleType::STATIC_RECTANGLE, id);
    
    MarbleManiaObstacle* obstaclePtr = obstacle.get();
    m_obstacles.push_back(std::move(obstacle));
    
    return obstaclePtr;
}

MarbleManiaObstacle* MarbleManiaMap::AddMovingCircle(const Vector2D& position, float radius, const Vector2D& direction, float speed, float distance) {
    // Convert to meters
    Vector2D pos(position.x / 100.0f, position.y / 100.0f);
    float r = radius / 100.0f;
    
    b2Body* body = CreateCircleBody(pos, r, true); // Static but we'll move manually
    
    b2Fixture* fixture = body->GetFixtureList();
    fixture->SetRestitution(0.8f);
    
    std::string id = GenerateObstacleId("moving_circle", position);
    auto obstacle = std::make_unique<MarbleManiaObstacle>(body, ObstacleType::MOVING_CIRCLE, id);
    obstacle->SetMovementPattern(direction, speed, distance); // Keep in pixels for movement logic
    
    MarbleManiaObstacle* obstaclePtr = obstacle.get();
    m_obstacles.push_back(std::move(obstacle));
    
    return obstaclePtr;
}

MarbleManiaObstacle* MarbleManiaMap::AddMovingRectangle(const Vector2D& position, float width, float height, const Vector2D& direction, float speed, float distance) {
    // Convert to meters
    Vector2D pos(position.x / 100.0f, position.y / 100.0f);
    float w = width / 100.0f;
    float h = height / 100.0f;
    
    b2Body* body = CreateRectangleBody(pos, w, h, 0.0f, true);
    
    b2Fixture* fixture = body->GetFixtureList();
    fixture->SetRestitution(0.8f);
    
    std::string id = GenerateObstacleId("moving_rect", position);
    auto obstacle = std::make_unique<MarbleManiaObstacle>(body, ObstacleType::MOVING_RECTANGLE, id);
    obstacle->SetMovementPattern(direction, speed, distance); // Keep in pixels for movement logic
    
    MarbleManiaObstacle* obstaclePtr = obstacle.get();
    m_obstacles.push_back(std::move(obstacle));
    
    return obstaclePtr;
}

MarbleManiaObstacle* MarbleManiaMap::AddSpinningCircle(const Vector2D& position, float radius, float rotationSpeed) {
    // Convert to meters
    Vector2D pos(position.x / 100.0f, position.y / 100.0f);
    float r = radius / 100.0f;
    
    b2Body* body = CreateCircleBody(pos, r, true);
    
    b2Fixture* fixture = body->GetFixtureList();
    fixture->SetRestitution(0.8f);
    
    std::string id = GenerateObstacleId("spinning_circle", position);
    auto obstacle = std::make_unique<MarbleManiaObstacle>(body, ObstacleType::SPINNING_CIRCLE, id);
    obstacle->SetRotationSpeed(rotationSpeed);
    
    MarbleManiaObstacle* obstaclePtr = obstacle.get();
    m_obstacles.push_back(std::move(obstacle));
    
    return obstaclePtr;
}

MarbleManiaObstacle* MarbleManiaMap::AddSpinningRectangle(const Vector2D& position, float width, float height, float rotationSpeed) {
    // Convert to meters
    Vector2D pos(position.x / 100.0f, position.y / 100.0f);
    float w = width / 100.0f;
    float h = height / 100.0f;
    
    b2Body* body = CreateRectangleBody(pos, w, h, 0.0f, true);
    
    b2Fixture* fixture = body->GetFixtureList();
    fixture->SetRestitution(0.8f);
    
    std::string id = GenerateObstacleId("spinning_rect", position);
    auto obstacle = std::make_unique<MarbleManiaObstacle>(body, ObstacleType::SPINNING_RECTANGLE, id);
    obstacle->SetRotationSpeed(rotationSpeed);
    
    MarbleManiaObstacle* obstaclePtr = obstacle.get();
    m_obstacles.push_back(std::move(obstacle));
    
    return obstaclePtr;
}

void MarbleManiaMap::CreateDefaultObstacles() {
    // Create some example obstacles to showcase the physics
    
    // Static circle obstacles
    AddStaticCircle(Vector2D(-100, 50), 30);
    AddStaticCircle(Vector2D(100, 150), 25);
    
    // Static rectangle platforms
    AddStaticRectangle(Vector2D(0, 200), 200, 20, 0.0f);
    AddStaticRectangle(Vector2D(-200, 100), 100, 15, 0.3f); // Slightly rotated
    AddStaticRectangle(Vector2D(200, 100), 100, 15, -0.3f); // Rotated other way
    
    // Moving obstacle
    AddMovingRectangle(Vector2D(0, 250), 150, 20, Vector2D(1, 0), 100.0f, 200.0f);
    
    // Spinning obstacles
    AddSpinningRectangle(Vector2D(-150, -100), 400, 15, 2.0f); // Spinning beam
    AddSpinningRectangle(Vector2D(150, -100), 400, 15, -1.5f); // Counter-rotating beam
    
    // Some circular obstacles for variety
    AddStaticCircle(Vector2D(0, 100), 20);
}

bool MarbleManiaMap::IsValidDropPosition(const Vector2D& position) const {
    return (position.x >= m_dropZoneMin.x && position.x <= m_dropZoneMax.x &&
            position.y >= m_dropZoneMin.y && position.y <= m_dropZoneMax.y);
}

bool MarbleManiaMap::IsPositionOccupied(const Vector2D& position, float radius, const MarbleManiaMarble* excludeMarble) const {
    // Check against other marbles
    for (const auto& pair : m_playerMarbles) {
        const MarbleManiaMarble* marble = pair.second.get();
        if (marble == excludeMarble) continue;
        
        Vector2D distance = marble->GetPosition() - position;
        float minDistance = marble->GetRadius() + radius + 5.0f; // Small buffer
        
        if (distance.Length() < minDistance) {
            return true;
        }
    }
    
    return false;
}

std::vector<std::pair<Client*, float>> MarbleManiaMap::GetFinishedMarbles() const {
    std::vector<std::pair<Client*, float>> finished;
    
    for (const auto& pair : m_playerMarbles) {
        if (pair.second->HasFinished()) {
            finished.push_back({pair.first, pair.second->GetTimeToFinish()});
        }
    }
    
    return finished;
}

std::vector<std::pair<Client*, int>> MarbleManiaMap::GetFinalPlacements() const {
    std::vector<std::pair<Client*, int>> placements;
    
    for (const auto& pair : m_playerMarbles) {
        int placement = pair.second->GetPlacement();
        if (placement == 0) {
            placement = m_nextPlacement; // Assign last place to unfinished marbles
        }
        placements.push_back({pair.first, placement});
    }
    
    // Sort by placement
    std::sort(placements.begin(), placements.end(), 
        [](const std::pair<Client*, int>& a, const std::pair<Client*, int>& b) {
            return a.second < b.second;
        });
    
    return placements;
}

std::string MarbleManiaMap::GenerateObstacleId(const std::string& type, const Vector2D& position) const {
    std::stringstream ss;
    ss << type << "_" << (int)position.x << "_" << (int)position.y;
    return ss.str();
}

void MarbleManiaMap::CreateBoundaries() {
    // Convert world bounds to meters
    float left = m_worldMin.x / 100.0f;
    float right = m_worldMax.x / 100.0f;
    float top = m_worldMin.y / 100.0f;
    float bottom = m_worldMax.y / 100.0f;
    float thickness = 0.1f; // 10cm thick boundaries
    
    // Left wall
    b2Body* leftWall = CreateRectangleBody(
        Vector2D(left - thickness/2, (top + bottom) / 2),
        thickness, bottom - top, 0.0f, true
    );
    m_boundaryBodies.push_back(leftWall);
    
    // Right wall
    b2Body* rightWall = CreateRectangleBody(
        Vector2D(right + thickness/2, (top + bottom) / 2),
        thickness, bottom - top, 0.0f, true
    );
    m_boundaryBodies.push_back(rightWall);
    
    // Bottom wall
    b2Body* bottomWall = CreateRectangleBody(
        Vector2D((left + right) / 2, bottom + thickness/2),
        right - left, thickness, 0.0f, true
    );
    m_boundaryBodies.push_back(bottomWall);
    
    // Top wall (optional, comment out if you want marbles to fall from above)
    // b2Body* topWall = CreateRectangleBody(
    //     Vector2D((left + right) / 2, top - thickness/2),
    //     right - left, thickness, 0.0f, true
    // );
    // m_boundaryBodies.push_back(topWall);
}

b2Body* MarbleManiaMap::CreateCircleBody(const Vector2D& position, float radius, bool isStatic) {
    b2BodyDef bodyDef;
    bodyDef.type = isStatic ? b2_staticBody : b2_dynamicBody;
    bodyDef.position.Set(position.x, position.y);
    
    b2Body* body = m_world->CreateBody(&bodyDef);
    
    b2CircleShape circleShape;
    circleShape.m_radius = radius;
    
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &circleShape;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    fixtureDef.restitution = 0.6f;
    
    body->CreateFixture(&fixtureDef);
    
    return body;
}

b2Body* MarbleManiaMap::CreateRectangleBody(const Vector2D& position, float width, float height, float rotation, bool isStatic) {
    b2BodyDef bodyDef;
    bodyDef.type = isStatic ? b2_staticBody : b2_dynamicBody;
    bodyDef.position.Set(position.x, position.y);
    bodyDef.angle = rotation;
    
    b2Body* body = m_world->CreateBody(&bodyDef);
    
    b2PolygonShape boxShape;
    boxShape.SetAsBox(width / 2.0f, height / 2.0f);
    
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &boxShape;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    fixtureDef.restitution = 0.6f;
    
    body->CreateFixture(&fixtureDef);
    
    return body;
}
