#include "teambased_pong_map.h"
#include <cmath>
#include <random>

TeambasedPong_Map::TeambasedPong_Map() 
    : world_(b2Vec2(0.0f, 0.0f)) { // No gravity for pong
}

TeambasedPong_Map::~TeambasedPong_Map() {
    // Box2D world cleanup is automatic
}

void TeambasedPong_Map::Initialize() {
    CreateWalls();
    CreatePaddles();
    CreateBall();
}

void TeambasedPong_Map::CreateWalls() {
    // Top wall
    b2BodyDef topWallDef;
    topWallDef.type = b2_staticBody;
    topWallDef.position.Set(0.0f, ToPhysicsScale(MAP_HEIGHT / 2.0f));
    topWall_ = world_.CreateBody(&topWallDef);
    
    b2PolygonShape topWallShape;
    topWallShape.SetAsBox(ToPhysicsScale(MAP_WIDTH / 2.0f), ToPhysicsScale(5.0f));
    
    b2FixtureDef topWallFixture;
    topWallFixture.shape = &topWallShape;
    topWallFixture.restitution = 1.0f; // Perfect bounce
    topWallFixture.friction = 0.0f;
    topWall_->CreateFixture(&topWallFixture);
    
    // Bottom wall
    b2BodyDef bottomWallDef;
    bottomWallDef.type = b2_staticBody;
    bottomWallDef.position.Set(0.0f, ToPhysicsScale(-MAP_HEIGHT / 2.0f));
    bottomWall_ = world_.CreateBody(&bottomWallDef);
    
    b2PolygonShape bottomWallShape;
    bottomWallShape.SetAsBox(ToPhysicsScale(MAP_WIDTH / 2.0f), ToPhysicsScale(5.0f));
    
    b2FixtureDef bottomWallFixture;
    bottomWallFixture.shape = &bottomWallShape;
    bottomWallFixture.restitution = 1.0f;
    bottomWallFixture.friction = 0.0f;
    bottomWall_->CreateFixture(&bottomWallFixture);
}

void TeambasedPong_Map::CreatePaddles() {
    // Paddle A (left side)
    b2BodyDef paddleADef;
    paddleADef.type = b2_kinematicBody; // Controlled by input, not physics
    paddleADef.position.Set(ToPhysicsScale(-MAP_WIDTH / 2.0f + 30.0f), 0.0f);
    paddleA_ = world_.CreateBody(&paddleADef);
    
    b2PolygonShape paddleAShape;
    paddleAShape.SetAsBox(ToPhysicsScale(PADDLE_WIDTH / 2.0f), ToPhysicsScale(PADDLE_HEIGHT / 2.0f));
    
    b2FixtureDef paddleAFixture;
    paddleAFixture.shape = &paddleAShape;
    paddleAFixture.restitution = 1.0f;
    paddleAFixture.friction = 0.0f;
    paddleA_->CreateFixture(&paddleAFixture);
    
    // Paddle B (right side)
    b2BodyDef paddleBDef;
    paddleBDef.type = b2_kinematicBody;
    paddleBDef.position.Set(ToPhysicsScale(MAP_WIDTH / 2.0f - 30.0f), 0.0f);
    paddleB_ = world_.CreateBody(&paddleBDef);
    
    b2PolygonShape paddleBShape;
    paddleBShape.SetAsBox(ToPhysicsScale(PADDLE_WIDTH / 2.0f), ToPhysicsScale(PADDLE_HEIGHT / 2.0f));
    
    b2FixtureDef paddleBFixture;
    paddleBFixture.shape = &paddleBShape;
    paddleBFixture.restitution = 1.0f;
    paddleBFixture.friction = 0.0f;
    paddleB_->CreateFixture(&paddleBFixture);
}

void TeambasedPong_Map::CreateBall() {
    b2BodyDef ballDef;
    ballDef.type = b2_dynamicBody;
    ballDef.position.Set(0.0f, 0.0f);
    ballDef.bullet = true; // Enable CCD for fast-moving ball
    ball_ = world_.CreateBody(&ballDef);
    
    b2CircleShape ballShape;
    ballShape.m_radius = ToPhysicsScale(BALL_RADIUS);
    
    b2FixtureDef ballFixture;
    ballFixture.shape = &ballShape;
    ballFixture.restitution = 1.0f; // Perfect bounce
    ballFixture.friction = 0.0f;
    ballFixture.density = 1.0f;
    ball_->CreateFixture(&ballFixture);
    
    ball_->SetLinearDamping(0.0f); // No damping, constant speed
    
    ResetBall();
}

void TeambasedPong_Map::ResetBall() {
    ball_->SetTransform(b2Vec2(0.0f, 0.0f), 0.0f);
    
    // Random angle between -45 and 45 degrees, going left or right
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angle_dist(-0.785f, 0.785f); // -45 to 45 degrees in radians
    std::uniform_int_distribution<int> direction_dist(0, 1);
    
    float angle = angle_dist(gen);
    float direction = direction_dist(gen) == 0 ? 1.0f : -1.0f;
    
    float vx = std::cos(angle) * ToPhysicsScale(BALL_INITIAL_SPEED) * direction;
    float vy = std::sin(angle) * ToPhysicsScale(BALL_INITIAL_SPEED);
    
    ball_->SetLinearVelocity(b2Vec2(vx, vy));
    current_ball_speed_ = BALL_INITIAL_SPEED;
}

void TeambasedPong_Map::Update(float dt) {
    // Update paddle positions based on team input
    if (paddleA_) {
        b2Vec2 currentPos = paddleA_->GetPosition();
        float targetY = paddleA_target_y_ + teamA_input_ * ToPhysicsScale(PADDLE_SPEED) * dt;
        
        // Clamp to map boundaries
        float maxY = ToPhysicsScale(MAP_HEIGHT / 2.0f - PADDLE_HEIGHT / 2.0f - 10.0f);
        targetY = std::max(-maxY, std::min(maxY, targetY));
        paddleA_target_y_ = targetY;
        
        paddleA_->SetTransform(b2Vec2(currentPos.x, targetY), 0.0f);
    }
    
    if (paddleB_) {
        b2Vec2 currentPos = paddleB_->GetPosition();
        float targetY = paddleB_target_y_ + teamB_input_ * ToPhysicsScale(PADDLE_SPEED) * dt;
        
        // Clamp to map boundaries
        float maxY = ToPhysicsScale(MAP_HEIGHT / 2.0f - PADDLE_HEIGHT / 2.0f - 10.0f);
        targetY = std::max(-maxY, std::min(maxY, targetY));
        paddleB_target_y_ = targetY;
        
        paddleB_->SetTransform(b2Vec2(currentPos.x, targetY), 0.0f);
    }
    
    // Check for paddle collisions and increase ball speed
    if (ball_) {
        b2Vec2 velocity = ball_->GetLinearVelocity();
        float speed = velocity.Length();
        
        // If ball speed changed significantly (collision), increase it
        float worldSpeed = ToWorldScale(speed);
        if (std::abs(worldSpeed - current_ball_speed_) > 10.0f) {
            current_ball_speed_ = std::min(BALL_MAX_SPEED, worldSpeed * BALL_SPEED_INCREASE);
            
            // Normalize and apply new speed
            velocity.Normalize();
            velocity.x *= ToPhysicsScale(current_ball_speed_);
            velocity.y *= ToPhysicsScale(current_ball_speed_);
            ball_->SetLinearVelocity(velocity);
        }
    }
    
    // Step the physics world
    world_.Step(dt, 8, 3); // velocityIterations=8, positionIterations=3
}

void TeambasedPong_Map::SetTeamInput(Team team, float input_y) {
    // Clamp input to -1 to 1
    input_y = std::max(-1.0f, std::min(1.0f, input_y));
    
    if (team == Team::TEAM_A) {
        teamA_input_ = input_y;
    } else if (team == Team::TEAM_B) {
        teamB_input_ = input_y;
    }
}

Vector2D TeambasedPong_Map::GetPaddlePosition(Team team) const {
    if (team == Team::TEAM_A && paddleA_) {
        return ToWorldCoords(Vector2D(paddleA_->GetPosition().x, paddleA_->GetPosition().y));
    } else if (team == Team::TEAM_B && paddleB_) {
        return ToWorldCoords(Vector2D(paddleB_->GetPosition().x, paddleB_->GetPosition().y));
    }
    return Vector2D(0.0f, 0.0f);
}

Vector2D TeambasedPong_Map::GetBallPosition() const {
    if (ball_) {
        return ToWorldCoords(Vector2D(ball_->GetPosition().x, ball_->GetPosition().y));
    }
    return Vector2D(0.0f, 0.0f);
}

Vector2D TeambasedPong_Map::GetBallVelocity() const {
    if (ball_) {
        b2Vec2 vel = ball_->GetLinearVelocity();
        return ToWorldCoords(Vector2D(vel.x, vel.y));
    }
    return Vector2D(0.0f, 0.0f);
}

Team TeambasedPong_Map::GetScoringTeam() const {
    if (!ball_) return Team::SPECTATOR;
    
    b2Vec2 ballPos = ball_->GetPosition();
    float ballX = ToWorldScale(ballPos.x);
    
    // Check if ball went past left paddle (Team B scores)
    if (ballX < -MAP_WIDTH / 2.0f) {
        return Team::TEAM_B;
    }
    
    // Check if ball went past right paddle (Team A scores)
    if (ballX > MAP_WIDTH / 2.0f) {
        return Team::TEAM_A;
    }
    
    return Team::SPECTATOR;
}

Vector2D TeambasedPong_Map::ToPhysicsCoords(const Vector2D& worldCoords) const {
    return Vector2D(worldCoords.x / PHYSICS_SCALE, worldCoords.y / PHYSICS_SCALE);
}

Vector2D TeambasedPong_Map::ToWorldCoords(const Vector2D& physicsCoords) const {
    return Vector2D(physicsCoords.x * PHYSICS_SCALE, physicsCoords.y * PHYSICS_SCALE);
}

float TeambasedPong_Map::ToPhysicsScale(float worldValue) const {
    return worldValue / PHYSICS_SCALE;
}

float TeambasedPong_Map::ToWorldScale(float physicsValue) const {
    return physicsValue * PHYSICS_SCALE;
}
