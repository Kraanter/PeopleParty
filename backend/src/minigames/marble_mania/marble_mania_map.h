#ifndef PEOPLEPARTY_BACKEND_MARBLE_MANIA_MAP_H
#define PEOPLEPARTY_BACKEND_MARBLE_MANIA_MAP_H

#include <vector>
#include <map>
#include <memory>
#include <string>
#include <box2d/box2d.h>
#include "../../client.h"
#include "../../util/math/vector2d.h"

#include "marble_mania_marble.h"
#include "marble_mania_obstacle.h"

// forward declaration
class Client;
class MarbleManiaMarble;
class MarbleManiaObstacle;


enum class GamePhase : uint8_t { PLACEMENT = 0, SIMULATION = 1, FINISHED = 2 };

class MarbleManiaMap {
public:
    MarbleManiaMap(const Vector2D& worldMin, const Vector2D& worldMax, float finishLineY, float finishLineOffset = 10.0f);
    ~MarbleManiaMap();

    // Scaling factor for Box2D optimization (divide by this for physics, multiply for frontend)
    static constexpr float PHYSICS_SCALE = 100.0f;

    // Players
    void CreatePlayers(const std::vector<Client*>& clients);
    void CreatePlayers(const std::map<int, Client*>& clients);
    void CreatePlayers(const std::map<int, std::unique_ptr<Client>>& clients);

    // Placement: x,y are joystick deltas in [-1..1] (relative motion)
    void MovePlayerMarble(Client* client, float x, float y);
    void SetPlayerReady(Client* client, bool ready);
    bool AllPlayersReady() const;

    // Simulation
    void StartPhysicsSimulation();
    void Update(float dt);

    // Accessors
    GamePhase GetCurrentPhase() const { return phase_; }
    const std::map<Client*, std::unique_ptr<MarbleManiaMarble>>& GetPlayerMarbles() const { return marbles_; }
    const std::vector<std::unique_ptr<MarbleManiaObstacle>>& GetObstacles() const { return obstacles_; }
    float GetFinishLine() const { return finishLineY_; }

    Vector2D GetWorldMin() const { return worldMin_; }
    Vector2D GetWorldMax() const { return worldMax_; }

    // Results
    std::vector<std::pair<Client*, int>> GetFinalPlacements() const { return finalPlacements_; }

    // Helper functions for coordinate scaling
    Vector2D ToPhysicsCoords(const Vector2D& worldCoords) const;
    Vector2D ToWorldCoords(const Vector2D& physicsCoords) const;
    float ToPhysicsScale(float worldValue) const;
    float ToWorldScale(float physicsValue) const;

private:
    void createWorldBounds_();
    void spawnDefaultObstacles_(); // example layout
    void computeFinalPlacements_();
    bool everyoneFinished_() const;

    Vector2D clampToDropZone_(const Vector2D& p) const;

private:
    b2World world_;
    Vector2D worldMin_;
    Vector2D worldMax_;
    float finishLineY_;
    float finishLineOffset_;

    GamePhase phase_ = GamePhase::PLACEMENT;

    std::map<Client*, std::unique_ptr<MarbleManiaMarble>> marbles_;
    std::map<Client*, bool> ready_;

    std::vector<std::unique_ptr<MarbleManiaObstacle>> obstacles_;

    float simTime_ = 0.0f;
    float maxSimTime_ = 60.0f;

    std::vector<std::pair<Client*, int>> finalPlacements_;
};

#endif // PEOPLEPARTY_BACKEND_MARBLE_MANIA_MAP_H
