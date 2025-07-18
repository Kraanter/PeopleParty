#ifndef MARBLE_MANIA_MAP_H
#define MARBLE_MANIA_MAP_H

#include <map>
#include <vector>
#include <memory>
#include "../../util/physics/physics_world.h"
#include "../../util/physics/spatial_grid.h"
#include "marble_mania_marble.h"
#include "marble_mania_obstacle.h"
#include "../../client.h"

enum class GamePhase {
    PLACEMENT,
    PHYSICS_SIMULATION,
    FINISHED
};

class MarbleManiaMarble;

class MarbleManiaMap {
private:
    std::unique_ptr<PhysicsWorld> m_physicsWorld;
    std::unique_ptr<SpatialGrid> m_spatialGrid;
    
    // Game objects
    std::map<Client*, std::unique_ptr<MarbleManiaMarble>> m_playerMarbles;
    std::vector<std::unique_ptr<MarbleManiaObstacle>> m_obstacles;
    
    // Game state
    GamePhase m_currentPhase;
    float m_gameTime;
    float m_finishLine;
    
    // Map properties
    Vector2D m_worldMin;
    Vector2D m_worldMax;
    Vector2D m_dropZoneMin;
    Vector2D m_dropZoneMax;
    
    // Placement tracking
    std::map<Client*, bool> m_playersReady;
    int m_finishedMarbles;
    
public:
    MarbleManiaMap(const Vector2D& worldMin, const Vector2D& worldMax, float finishLine);
    ~MarbleManiaMap();
    
    // Game phase management
    void SetPhase(GamePhase phase) { m_currentPhase = phase; }
    GamePhase GetCurrentPhase() const { return m_currentPhase; }
    
    // Player management
    void CreatePlayers(const std::vector<Client*>& clients);
    void RemovePlayer(Client* client);
    bool PlaceMarble(Client* client, const Vector2D& position);
    void MovePlayerMarble(Client* client, float deltaX, float deltaY);
    void SetPlayerReady(Client* client, bool ready);
    bool IsPlayerReady(Client* client) const;
    bool AllPlayersReady() const;
    
    // Game simulation
    void Update(float deltaTime);
    void StartPhysicsSimulation();
    
    // Obstacle management
    void CreateObstacles();
    void AddStaticCircle(const Vector2D& position, float radius);
    void AddStaticRectangle(const Vector2D& position, float width, float height);
    void AddMovingCircle(const Vector2D& position, float radius, const Vector2D& direction, float speed, float distance);
    void AddMovingRectangle(const Vector2D& position, float width, float height, const Vector2D& direction, float speed, float distance);
    void AddSpinningCircle(const Vector2D& position, float radius, float rotationSpeed);
    void AddSpinningRectangle(const Vector2D& position, float width, float height, float rotationSpeed);
    
    // Results
    std::vector<std::pair<Client*, float>> GetFinishedMarbles() const;
    std::vector<std::pair<Client*, int>> GetFinalPlacements() const;
    
    // Getters
    const std::map<Client*, std::unique_ptr<MarbleManiaMarble>>& GetPlayerMarbles() const { return m_playerMarbles; }
    const std::vector<std::unique_ptr<MarbleManiaObstacle>>& GetObstacles() const { return m_obstacles; }
    float GetGameTime() const { return m_gameTime; }
    float GetFinishLine() const { return m_finishLine; }
    
    // Drop zone validation
    bool IsValidDropPosition(const Vector2D& position) const;
    const Vector2D& GetDropZoneMin() const { return m_dropZoneMin; }
    const Vector2D& GetDropZoneMax() const { return m_dropZoneMax; }
    
private:
    void UpdatePhysics(float deltaTime);
    void UpdateObstacles(float deltaTime);
    void CheckFinishLine();
    void UpdatePlacements();
    bool IsPositionOccupied(const Vector2D& position, float radius) const;
    bool IsPositionOccupied(const Vector2D& position, float radius, const MarbleManiaMarble* excludeMarble) const;
    void CreateDefaultObstacles();
};

#endif // MARBLE_MANIA_MAP_H
