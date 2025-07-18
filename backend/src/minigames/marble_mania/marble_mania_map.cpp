#include "marble_mania_map.h"
#include <algorithm>
#include <cmath>

MarbleManiaMap::MarbleManiaMap(const Vector2D& worldMin, const Vector2D& worldMax, float finishLine)
    : m_currentPhase(GamePhase::PLACEMENT)
    , m_gameTime(0.0f)
    , m_finishLine(finishLine)
    , m_worldMin(worldMin)
    , m_worldMax(worldMax)
    , m_finishedMarbles(0)
{
    // Create physics world
    m_physicsWorld = std::make_unique<PhysicsWorld>(worldMin, worldMax);
    m_physicsWorld->SetGravity(Vector2D(0, 500.0f)); // Gravity pointing down
    
    // Create spatial grid for optimization
    float cellSize = 50.0f;
    m_spatialGrid = std::make_unique<SpatialGrid>(cellSize, worldMin, worldMax);
    
    // Set up drop zone (top area of the map)
    m_dropZoneMin = Vector2D(worldMin.x + 50, worldMin.y);
    m_dropZoneMax = Vector2D(worldMax.x - 50, worldMin.y + 100);
    
    // Add world boundaries
    m_physicsWorld->AddBoundary(Vector2D(worldMin.x, worldMax.y), Vector2D(worldMax.x, worldMax.y)); // Floor
    m_physicsWorld->AddBoundary(Vector2D(worldMin.x, worldMin.y), Vector2D(worldMin.x, worldMax.y)); // Left wall
    m_physicsWorld->AddBoundary(Vector2D(worldMax.x, worldMin.y), Vector2D(worldMax.x, worldMax.y)); // Right wall
    
    // Create default obstacles
    CreateDefaultObstacles();
}

MarbleManiaMap::~MarbleManiaMap() {
    m_playerMarbles.clear();
    m_obstacles.clear();
}

void MarbleManiaMap::CreatePlayers(const std::vector<Client*>& clients) {
    // Calculate spacing for marbles in drop zone
    float dropZoneWidth = m_dropZoneMax.x - m_dropZoneMin.x;
    float spacing = dropZoneWidth / (clients.size() + 1);
    
    for (size_t i = 0; i < clients.size(); i++) {
        Client* client = clients[i];
        if (client && !client->isHost && m_playerMarbles.find(client) == m_playerMarbles.end()) {
            // Calculate initial position in drop zone
            float x = m_dropZoneMin.x + spacing * (i + 1);
            float y = m_dropZoneMin.y + 20; // A bit below the top of drop zone
            Vector2D initialPosition(x, y);
            
            // Create marble for player
            auto marble = std::make_unique<MarbleManiaMarble>(initialPosition, client);
            m_playerMarbles[client] = std::move(marble);
            m_playersReady[client] = false;
        }
    }
}

void MarbleManiaMap::RemovePlayer(Client* client) {
    auto it = m_playerMarbles.find(client);
    if (it != m_playerMarbles.end()) {
        m_playerMarbles.erase(it);
    }
    
    auto readyIt = m_playersReady.find(client);
    if (readyIt != m_playersReady.end()) {
        m_playersReady.erase(readyIt);
    }
}

bool MarbleManiaMap::PlaceMarble(Client* client, const Vector2D& position) {
    if (m_currentPhase != GamePhase::PLACEMENT) {
        return false;
    }
    
    if (!IsValidDropPosition(position)) {
        return false;
    }
    
    // Check if position is occupied
    if (IsPositionOccupied(position, 20.0f)) {
        return false;
    }
    
    // Create marble for player
    auto marble = std::make_unique<MarbleManiaMarble>(position, client);
    m_playerMarbles[client] = std::move(marble);
    m_playersReady[client] = true;
    
    return true;
}

bool MarbleManiaMap::IsPlayerReady(Client* client) const {
    auto it = m_playersReady.find(client);
    return it != m_playersReady.end() && it->second;
}

bool MarbleManiaMap::AllPlayersReady() const {
    for (const auto& pair : m_playersReady) {
        if (!pair.second) {
            return false;
        }
    }
    return !m_playersReady.empty();
}

void MarbleManiaMap::MovePlayerMarble(Client* client, float deltaX, float deltaY) {
    if (m_currentPhase != GamePhase::PLACEMENT) {
        return;
    }
    
    auto it = m_playerMarbles.find(client);
    if (it == m_playerMarbles.end()) {
        return;
    }
    
    MarbleManiaMarble* marble = it->second.get();
    Vector2D currentPos = marble->GetPosition();
    
    // Apply movement with sensitivity scaling
    float sensitivity = 2.0f;
    Vector2D newPos = currentPos + Vector2D(deltaX * sensitivity, deltaY * sensitivity);
    
    // Clamp to drop zone bounds
    newPos.x = std::max(m_dropZoneMin.x + 20.0f, std::min(m_dropZoneMax.x - 20.0f, newPos.x));
    newPos.y = std::max(m_dropZoneMin.y + 10.0f, std::min(m_dropZoneMax.y - 10.0f, newPos.y));
    
    // Check if position is occupied by another marble
    if (!IsPositionOccupied(newPos, 20.0f, marble)) {
        marble->SetPosition(newPos);
    }
}

void MarbleManiaMap::SetPlayerReady(Client* client, bool ready) {
    auto it = m_playersReady.find(client);
    if (it != m_playersReady.end()) {
        it->second = ready;
    }
}

void MarbleManiaMap::Update(float deltaTime) {
    m_gameTime += deltaTime;
    
    switch (m_currentPhase) {
        case GamePhase::PLACEMENT:
            // Nothing to update during placement
            break;
            
        case GamePhase::PHYSICS_SIMULATION:
            UpdatePhysics(deltaTime);
            UpdateObstacles(deltaTime);
            CheckFinishLine();
            
            // Update marble timers
            for (auto& pair : m_playerMarbles) {
                pair.second->UpdateTimer(deltaTime);
            }
            
            // Check if all marbles have finished
            if (m_finishedMarbles >= m_playerMarbles.size()) {
                m_currentPhase = GamePhase::FINISHED;
                UpdatePlacements();
            }
            break;
            
        case GamePhase::FINISHED:
            // Game is over
            break;
    }
}

void MarbleManiaMap::StartPhysicsSimulation() {
    if (m_currentPhase != GamePhase::PLACEMENT) {
        return;
    }
    
    m_currentPhase = GamePhase::PHYSICS_SIMULATION;
    m_gameTime = 0.0f;
    
    // Add all marbles to physics world
    for (auto& pair : m_playerMarbles) {
        MarbleManiaMarble* marble = pair.second.get();
        RigidBody* body = m_physicsWorld->CreateBody(marble->GetPosition(), marble->GetMass(), marble->GetRadius());
        // Copy properties from marble to physics body
        body->SetRestitution(marble->GetRestitution());
        body->SetFriction(marble->GetFriction());
    }
    
    // Add obstacles to physics world
    for (auto& obstacle : m_obstacles) {
        RigidBody* body = m_physicsWorld->CreateBody(obstacle->GetPosition(), obstacle->GetMass(), obstacle->GetRadius());
        body->SetRestitution(obstacle->GetRestitution());
        body->SetFriction(obstacle->GetFriction());
        body->SetStatic(obstacle->IsStatic());
    }
}

void MarbleManiaMap::AddStaticCircle(const Vector2D& position, float radius) {
    m_obstacles.push_back(std::unique_ptr<MarbleManiaObstacle>(
        MarbleManiaObstacle::CreateStaticCircle(position, radius)));
}

void MarbleManiaMap::AddStaticRectangle(const Vector2D& position, float width, float height) {
    m_obstacles.push_back(std::unique_ptr<MarbleManiaObstacle>(
        MarbleManiaObstacle::CreateStaticRectangle(position, width, height)));
}

void MarbleManiaMap::AddMovingCircle(const Vector2D& position, float radius, const Vector2D& direction, float speed, float distance) {
    m_obstacles.push_back(std::unique_ptr<MarbleManiaObstacle>(
        MarbleManiaObstacle::CreateMovingCircle(position, radius, direction, speed, distance)));
}

void MarbleManiaMap::AddMovingRectangle(const Vector2D& position, float width, float height, const Vector2D& direction, float speed, float distance) {
    m_obstacles.push_back(std::unique_ptr<MarbleManiaObstacle>(
        MarbleManiaObstacle::CreateMovingRectangle(position, width, height, direction, speed, distance)));
}

void MarbleManiaMap::AddSpinningCircle(const Vector2D& position, float radius, float rotationSpeed) {
    m_obstacles.push_back(std::unique_ptr<MarbleManiaObstacle>(
        MarbleManiaObstacle::CreateSpinningCircle(position, radius, rotationSpeed)));
}

void MarbleManiaMap::AddSpinningRectangle(const Vector2D& position, float width, float height, float rotationSpeed) {
    m_obstacles.push_back(std::unique_ptr<MarbleManiaObstacle>(
        MarbleManiaObstacle::CreateSpinningRectangle(position, width, height, rotationSpeed)));
}

std::vector<std::pair<Client*, float>> MarbleManiaMap::GetFinishedMarbles() const {
    std::vector<std::pair<Client*, float>> finished;
    
    for (const auto& pair : m_playerMarbles) {
        if (pair.second->HasFinished()) {
            finished.push_back({pair.first, pair.second->GetTimeToFinish()});
        }
    }
    
    // Sort by time to finish
    std::sort(finished.begin(), finished.end(), 
              [](const std::pair<Client*, float>& a, const std::pair<Client*, float>& b) {
                  return a.second < b.second;
              });
    
    return finished;
}

std::vector<std::pair<Client*, int>> MarbleManiaMap::GetFinalPlacements() const {
    std::vector<std::pair<Client*, int>> placements;
    
    for (const auto& pair : m_playerMarbles) {
        placements.push_back({pair.first, pair.second->GetPlacement()});
    }
    
    return placements;
}

bool MarbleManiaMap::IsValidDropPosition(const Vector2D& position) const {
    return position.x >= m_dropZoneMin.x && position.x <= m_dropZoneMax.x &&
           position.y >= m_dropZoneMin.y && position.y <= m_dropZoneMax.y;
}

void MarbleManiaMap::UpdatePhysics(float deltaTime) {
    m_physicsWorld->Step(deltaTime);
    
    // Update marble positions from physics world
    const auto& bodies = m_physicsWorld->GetBodies();
    int marbleIndex = 0;
    
    for (auto& pair : m_playerMarbles) {
        if (marbleIndex < bodies.size()) {
            pair.second->SetPosition(bodies[marbleIndex]->GetPosition());
            pair.second->SetVelocity(bodies[marbleIndex]->GetVelocity());
            marbleIndex++;
        }
    }
}

void MarbleManiaMap::UpdateObstacles(float deltaTime) {
    for (auto& obstacle : m_obstacles) {
        obstacle->Update(deltaTime); // Use the combined Update method
    }
}

void MarbleManiaMap::CheckFinishLine() {
    for (auto& pair : m_playerMarbles) {
        if (!pair.second->HasFinished()) {
            pair.second->CheckFinishLine(m_finishLine);
            if (pair.second->HasFinished()) {
                m_finishedMarbles++;
            }
        }
    }
}

void MarbleManiaMap::UpdatePlacements() {
    auto finished = GetFinishedMarbles();
    
    for (int i = 0; i < finished.size(); ++i) {
        auto it = m_playerMarbles.find(finished[i].first);
        if (it != m_playerMarbles.end()) {
            it->second->SetPlacement(i + 1);
        }
    }
    
    // Set placement for unfinished marbles
    int unfinishedPlacement = finished.size() + 1;
    for (auto& pair : m_playerMarbles) {
        if (!pair.second->HasFinished()) {
            pair.second->SetPlacement(unfinishedPlacement);
        }
    }
}

bool MarbleManiaMap::IsPositionOccupied(const Vector2D& position, float radius) const {
    return IsPositionOccupied(position, radius, nullptr);
}

bool MarbleManiaMap::IsPositionOccupied(const Vector2D& position, float radius, const MarbleManiaMarble* excludeMarble) const {
    for (const auto& pair : m_playerMarbles) {
        if (pair.second.get() != excludeMarble && 
            pair.second->GetPosition().Distance(position) < radius + pair.second->GetRadius()) {
            return true;
        }
    }
    return false;
}

void MarbleManiaMap::CreateDefaultObstacles() {
    // Create a variety of obstacles
    
    // Static circles
    AddStaticCircle(Vector2D(-100, -50), 25);
    AddStaticCircle(Vector2D(100, -50), 25);
    AddStaticCircle(Vector2D(0, 100), 30);
    AddStaticCircle(Vector2D(-150, 200), 20);
    AddStaticCircle(Vector2D(150, 200), 20);
    
    // Static rectangles
    AddStaticRectangle(Vector2D(-50, 150), 100, 20);
    AddStaticRectangle(Vector2D(50, 250), 80, 15);
    
    // Moving circles
    AddMovingCircle(Vector2D(-200, 100), 15, Vector2D(1, 0), 50, 100);
    AddMovingCircle(Vector2D(200, 150), 15, Vector2D(-1, 0), 40, 80);
    
    // Moving rectangles
    AddMovingRectangle(Vector2D(0, 300), 60, 15, Vector2D(1, 0), 30, 120);
    
    // Spinning circles
    AddSpinningCircle(Vector2D(-75, 75), 20, 2.0f); // 2 rad/s rotation
    AddSpinningCircle(Vector2D(75, 175), 18, -1.5f); // Counter-clockwise
    
    // Spinning rectangles  
    AddSpinningRectangle(Vector2D(-25, 325), 50, 12, 1.0f);
    AddSpinningRectangle(Vector2D(25, 225), 40, 10, -2.5f);
}
