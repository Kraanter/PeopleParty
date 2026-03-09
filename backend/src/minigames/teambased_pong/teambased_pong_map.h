#ifndef PEOPLEPARTY_BACKEND_TEAMBASED_PONG_MAP_H
#define PEOPLEPARTY_BACKEND_TEAMBASED_PONG_MAP_H

#include <box2d/box2d.h>
#include <vector>
#include "../../util/math/vector2d.h"
#include "teambased_pong_player.h"

// Intercepts ball collisions in PreSolve so we can disable Box2D's built-in impulse
// and apply our own clean velocity inversion after world_.Step().
class PongContactListener : public b2ContactListener {
public:
    b2Body* ball     = nullptr;
    b2Body* paddleA  = nullptr;
    b2Body* paddleB  = nullptr;
    b2Body* topWall  = nullptr;
    b2Body* bottomWall = nullptr;

    // Set by PreSolve, consumed and reset by Update()
    bool hit_left_paddle  = false;
    bool hit_right_paddle = false;
    bool hit_top_wall     = false;
    bool hit_bottom_wall  = false;
    b2Vec2 ball_vel_at_contact{0.0f, 0.0f}; // ball velocity captured before Box2D runs

    void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
};

class TeambasedPong_Map {
public:
    TeambasedPong_Map();
    ~TeambasedPong_Map();
    
    // Scaling factor for Box2D optimization (divide by this for physics, multiply for frontend)
    static constexpr float PHYSICS_SCALE = 100.0f;
    
    // Map dimensions (world coordinates)
    static constexpr float MAP_WIDTH = 800.0f;
    static constexpr float MAP_HEIGHT = 600.0f;
    
    // Paddle properties
    static constexpr float PADDLE_WIDTH = 20.0f;
    static constexpr float PADDLE_HEIGHT = 120.0f;
    static constexpr float PADDLE_SPEED = 300.0f; // units per second
    
    // Ball properties
    static constexpr float BALL_RADIUS = 10.0f;
    static constexpr float BALL_INITIAL_SPEED = 100.0f;
    static constexpr float BALL_SPEED_INCREASE = 1.02f; // 2% increase per bounce
    static constexpr float BALL_MAX_SPEED = 600.0f;
    
    // Initialize the map
    void Initialize();
    
    // Update physics simulation
    void Update(float dt);
    
    // Set paddle input for a team (-1 to 1)
    void SetTeamInput(Team team, float input_y);
    
    // Get paddle positions (world coordinates)
    Vector2D GetPaddlePosition(Team team) const;
    
    // Get ball position and velocity (world coordinates)
    Vector2D GetBallPosition() const;
    Vector2D GetBallVelocity() const;
    
    // Check if a team scored (ball went past paddle)
    Team GetScoringTeam() const; // Returns SPECTATOR if no score
    
    // Reset ball to center with random direction
    void ResetBall();
    
    // Helper functions for coordinate scaling
    Vector2D ToPhysicsCoords(const Vector2D& worldCoords) const;
    Vector2D ToWorldCoords(const Vector2D& physicsCoords) const;
    float ToPhysicsScale(float worldValue) const;
    float ToWorldScale(float physicsValue) const;
    
private:
    void CreateWalls();
    void CreatePaddles();
    void CreateBall();
    
    b2World world_;
    PongContactListener contact_listener_;

    // Physics bodies
    b2Body* paddleA_ = nullptr;
    b2Body* paddleB_ = nullptr;
    b2Body* ball_ = nullptr;
    b2Body* topWall_ = nullptr;
    b2Body* bottomWall_ = nullptr;
    
    // Paddle target positions (influenced by team input)
    float paddleA_target_y_ = 0.0f;
    float paddleB_target_y_ = 0.0f;
    
    // Team inputs
    float teamA_input_ = 0.0f;
    float teamB_input_ = 0.0f;
    
    // Ball speed tracking
    float current_ball_speed_ = BALL_INITIAL_SPEED;
};

#endif // PEOPLEPARTY_BACKEND_TEAMBASED_PONG_MAP_H
