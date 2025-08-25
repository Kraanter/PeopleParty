#ifndef MARBLE_MANIA_MAP_H
#define MARBLE_MANIA_MAP_H

#include <map>
#include <vector>
#include <memory>
#include <box2d/box2d.h>
#include "marble_mania_marble.h"
#include "marble_mania_obstacle.h"
#include "../../client.h"

enum class GamePhase {
    PLACEMENT = 0,
    PHYSICS_SIMULATION = 1,
    FINISHED = 2
};

class MarbleManiaMarble;
class MarbleManiaObstacle;

class MarbleManiaMap {
private:
    std::unique_ptr<b2World> m_world;
    
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
    int m_nextPlacement;
    
    // Box2D boundaries
    std::vector<b2Body*> m_boundaryBodies;
    
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
    
    // Obstacle management - factory methods
    void CreateObstacles();
    MarbleManiaObstacle* AddStaticCircle(const Vector2D& position, float radius);
    MarbleManiaObstacle* AddStaticRectangle(const Vector2D& position, float width, float height, float rotation = 0.0f);
    MarbleManiaObstacle* AddMovingCircle(const Vector2D& position, float radius, const Vector2D& direction, float speed, float distance);
    MarbleManiaObstacle* AddMovingRectangle(const Vector2D& position, float width, float height, const Vector2D& direction, float speed, float distance);
    MarbleManiaObstacle* AddSpinningCircle(const Vector2D& position, float radius, float rotationSpeed);
    MarbleManiaObstacle* AddSpinningRectangle(const Vector2D& position, float width, float height, float rotationSpeed);
    
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
    bool IsPositionOccupied(const Vector2D& position, float radius, const MarbleManiaMarble* excludeMarble = nullptr) const;
    void CreateDefaultObstacles();
    void CreateBoundaries();
    std::string GenerateObstacleId(const std::string& type, const Vector2D& position) const;
    
    // Box2D factory methods
    b2Body* CreateCircleBody(const Vector2D& position, float radius, bool isStatic = true);
    b2Body* CreateRectangleBody(const Vector2D& position, float width, float height, float rotation = 0.0f, bool isStatic = true);
};

#endif // MARBLE_MANIA_MAP_H
