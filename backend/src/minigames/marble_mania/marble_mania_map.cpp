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
    for (Client* client : clients) {
        if (client && m_playerMarbles.find(client) == m_playerMarbles.end()) {
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
        obstacle->UpdateMovement(deltaTime);
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
    for (const auto& pair : m_playerMarbles) {
        if (pair.second->GetPosition().Distance(position) < radius + pair.second->GetRadius()) {
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
}
