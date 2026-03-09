#include "teambased_pong_map.h"
#include <cmath>
#include <random>

void PongContactListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold) {
    if (!ball) return;

    b2Body* bodyA = contact->GetFixtureA()->GetBody();
    b2Body* bodyB = contact->GetFixtureB()->GetBody();

    bool ballIsA = (bodyA == ball);
    bool ballIsB = (bodyB == ball);
    if (!ballIsA && !ballIsB) return;

    // Disable Box2D’s impulse solver for this contact — we apply the bounce manually.
    contact->SetEnabled(false);

    b2Body* other = ballIsA ? bodyB : bodyA;

    // Capture velocity on the very first contact hit this step.
    bool already_registered = hit_left_paddle || hit_right_paddle || hit_top_wall || hit_bottom_wall;
    if (!already_registered) {
        ball_vel_at_contact = ball->GetLinearVelocity();
    }

    if      (other == paddleA)    hit_left_paddle  = true;
    else if (other == paddleB)    hit_right_paddle = true;
    else if (other == topWall)    hit_top_wall     = true;
    else if (other == bottomWall) hit_bottom_wall  = true;
}

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

    // Point the listener at our bodies and register it with the world.
    contact_listener_.ball       = ball_;
    contact_listener_.paddleA    = paddleA_;
    contact_listener_.paddleB    = paddleB_;
    contact_listener_.topWall    = topWall_;
    contact_listener_.bottomWall = bottomWall_;
    world_.SetContactListener(&contact_listener_);
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
    
    // Reset contact flags before stepping so each step starts clean.
    contact_listener_.hit_left_paddle  = false;
    contact_listener_.hit_right_paddle = false;
    contact_listener_.hit_top_wall     = false;
    contact_listener_.hit_bottom_wall  = false;

    world_.Step(dt, 8, 3); // velocityIterations=8, positionIterations=3

    // Apply manual velocity inversion based on what was hit this step.
    // PreSolve disabled Box2D’s impulse, so the ball velocity is still the
    // pre-contact value captured in ball_vel_at_contact.
    if (ball_) {
        b2Vec2 vel = contact_listener_.ball_vel_at_contact;
        bool hit_paddle = contact_listener_.hit_left_paddle || contact_listener_.hit_right_paddle;
        bool hit_wall   = contact_listener_.hit_top_wall    || contact_listener_.hit_bottom_wall;

        if (contact_listener_.hit_left_paddle) {
            vel.x =  std::abs(vel.x); // always bounce right away from left paddle
        } else if (contact_listener_.hit_right_paddle) {
            vel.x = -std::abs(vel.x); // always bounce left away from right paddle
        }

        if (contact_listener_.hit_top_wall) {
            vel.y = -std::abs(vel.y); // always bounce downward away from top wall
        } else if (contact_listener_.hit_bottom_wall) {
            vel.y =  std::abs(vel.y); // always bounce upward away from bottom wall
        }

        if (hit_paddle || hit_wall) {
            if (hit_paddle) {
                // Increase ball speed on every paddle bounce
                float newWorldSpeed = ToWorldScale(vel.Length()) * BALL_SPEED_INCREASE;
                current_ball_speed_ = std::min(BALL_MAX_SPEED, newWorldSpeed);
                vel.Normalize();
                vel *= ToPhysicsScale(current_ball_speed_);
            }
            ball_->SetLinearVelocity(vel);
        }
    }
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
