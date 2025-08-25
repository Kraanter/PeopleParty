#include "marble_mania_map.h"
#include <algorithm>
#include <sstream>
#include <cmath>

static b2Vec2 toB2(const Vector2D& v){ return b2Vec2(v.x, v.y); }
static Vector2D toV2(const b2Vec2& v){ return Vector2D{v.x, v.y}; }

MarbleManiaMap::MarbleManiaMap(const Vector2D& worldMin, const Vector2D& worldMax, float finishLineY)
: world_(b2Vec2(0.0f, 20.0f)) // gravity downward; set to (0,0) for full zero-gravity world
, worldMin_(worldMin)
, worldMax_(worldMax)
, finishLineY_(finishLineY)
{
    createWorldBounds_();
    spawnDefaultObstacles_();
}

MarbleManiaMap::~MarbleManiaMap() {
    // world_ owns bodies/joints; obstacle/marble wrappers clean up first
}

void MarbleManiaMap::createWorldBounds_() {
    // Static borders (edge shapes)
    b2BodyDef bd;
    b2Body* bounds = world_.CreateBody(&bd);

    b2EdgeShape edge;
    b2FixtureDef fd;
    fd.shape = &edge;
    fd.friction = 0.6f;
    fd.restitution = 0.3f;

    // bottom
    edge.SetTwoSided(b2Vec2(worldMin_.x, worldMax_.y), b2Vec2(worldMax_.x, worldMax_.y));
    bounds->CreateFixture(&fd);
    // top
    edge.SetTwoSided(b2Vec2(worldMin_.x, worldMin_.y), b2Vec2(worldMax_.x, worldMin_.y));
    bounds->CreateFixture(&fd);
    // left
    edge.SetTwoSided(b2Vec2(worldMin_.x, worldMin_.y), b2Vec2(worldMin_.x, worldMax_.y));
    bounds->CreateFixture(&fd);
    // right
    edge.SetTwoSided(b2Vec2(worldMax_.x, worldMin_.y), b2Vec2(worldMax_.x, worldMax_.y));
    bounds->CreateFixture(&fd);
}

void MarbleManiaMap::spawnDefaultObstacles_() {
    // Example static layout — tweak as desired.

    // Static circle bumper
    {
        obstacles_.push_back(std::make_unique<MarbleManiaObstacle>(
            world_, "obs_circle_1", ObstacleType::Circle, Vector2D{0.0f, -50.0f},
            60.0f, 60.0f, 0.0f));
    }
    // Static rectangle (can set a rotation angle)
    {
        obstacles_.push_back(std::make_unique<MarbleManiaObstacle>(
            world_, "obs_rect_1", ObstacleType::Rectangle, Vector2D{-150.0f, 50.0f},
            140.0f, 20.0f, 0.3f));
    }
    // Static triangle
    {
        obstacles_.push_back(std::make_unique<MarbleManiaObstacle>(
            world_, "obs_tri_1", ObstacleType::Triangle, Vector2D{120.0f, 120.0f},
            80.0f, 70.0f, 0.2f));
    }
}

void MarbleManiaMap::CreatePlayers(const std::vector<Client*>& clients) {
    marbles_.clear();
    ready_.clear();

    int idx = 0;
    const float cols = 6.0f;
    const float spacing = (worldMax_.x - worldMin_.x) / cols;

    for (Client* c : clients) {
        if (!c || c->isHost) continue;

        float x = worldMin_.x + spacing * (float)((idx % (int)cols) + 0.5f);
        float y = worldMin_.y + 40.0f;

        std::stringstream ss; ss << "marble_" << (idx + 1);
        marbles_[c] = std::make_unique<MarbleManiaMarble>(world_, ss.str(), Vector2D{x, y}, 10.0f);
        ready_[c] = false;
        ++idx;
    }
}

void MarbleManiaMap::CreatePlayers(const std::map<int, Client*>& clients) {
    marbles_.clear();
    ready_.clear();

    int idx = 0;
    const float cols = 6.0f;
    const float spacing = (worldMax_.x - worldMin_.x) / cols;

    for (const auto& kv : clients) {
        Client* c = kv.second;
        if (!c || c->isHost) continue;

        float x = worldMin_.x + spacing * (float)((idx % (int)cols) + 0.5f);
        float y = worldMin_.y + 40.0f;

        std::stringstream ss; ss << "marble_" << (idx + 1);
        marbles_[c] = std::make_unique<MarbleManiaMarble>(world_, ss.str(), Vector2D{x, y}, 10.0f);
        ready_[c] = false;
        ++idx;
    }
}

void MarbleManiaMap::CreatePlayers(const std::map<int, std::unique_ptr<Client>>& clients) {
    marbles_.clear();
    ready_.clear();

    int idx = 0;
    const float cols = 6.0f;
    const float spacing = (worldMax_.x - worldMin_.x) / cols;

    for (const auto& kv : clients) {
        Client* c = kv.second.get();
        if (!c || c->isHost) continue;

        float x = worldMin_.x + spacing * (float)((idx % (int)cols) + 0.5f);
        float y = worldMin_.y + 40.0f;

        std::stringstream ss; ss << "marble_" << (idx + 1);
        marbles_[c] = std::make_unique<MarbleManiaMarble>(world_, ss.str(), Vector2D{x, y}, 10.0f);
        ready_[c] = false;
        ++idx;
    }
}

Vector2D MarbleManiaMap::clampToDropZone_(const Vector2D& p) const {
    // Horizontal band near the top; adjust to taste
    float minY = worldMin_.y + 20.0f;
    float maxY = worldMin_.y + 120.0f;
    Vector2D out = p;
    out.x = std::max(worldMin_.x + 10.0f, std::min(worldMax_.x - 10.0f, out.x));
    out.y = std::max(minY, std::min(maxY, out.y));
    return out;
}

void MarbleManiaMap::MovePlayerMarble(Client* client, float x, float y) {
    if (phase_ != GamePhase::PLACEMENT) return;
    auto it = marbles_.find(client);
    if (it == marbles_.end()) return;

    // Treat (x,y) as joystick deltas in [-1..1]
    const float pixelsPerTick = 10.0f; // tune feel
    Vector2D cur = it->second->GetPosition();
    Vector2D next { cur.x + x * pixelsPerTick, cur.y + y * pixelsPerTick };
    next = clampToDropZone_(next);
    it->second->Teleport(next);
}

void MarbleManiaMap::SetPlayerReady(Client* client, bool ready) {
    if (phase_ != GamePhase::PLACEMENT) return;
    auto it = ready_.find(client);
    if (it != ready_.end()) it->second = ready;
}

bool MarbleManiaMap::AllPlayersReady() const {
    if (ready_.empty()) return false;
    for (const auto& kv : ready_) {
        if (!kv.second) return false;
    }
    return true;
}

void MarbleManiaMap::StartPhysicsSimulation() {
    if (phase_ != GamePhase::PLACEMENT) return;

    for (auto& kv : marbles_) {
        kv.second->EnableGravity(true);
        if (auto* b = kv.second->GetBody()) b->SetAwake(true);
    }

    phase_ = GamePhase::SIMULATION;
    simTime_ = 0.0f;
}

bool MarbleManiaMap::everyoneFinished_() const {
    for (const auto& kv : marbles_) {
        if (!kv.second->HasFinished()) return false;
    }
    return true;
}

void MarbleManiaMap::computeFinalPlacements_() {
    struct Row { Client* c; bool finished; float t; float y; };
    std::vector<Row> rows; rows.reserve(marbles_.size());

    for (const auto& kv : marbles_) {
        Client* c = kv.first;
        const auto& m = *kv.second;
        rows.push_back(Row{ c, m.HasFinished(), m.GetTimeToFinish(), m.GetPosition().y });
    }

    std::sort(rows.begin(), rows.end(), [](const Row& a, const Row& b){
        if (a.finished != b.finished) return a.finished > b.finished; // finished first
        if (a.finished) return a.t < b.t;                             // faster is better
        return a.y > b.y;                                             // farther down
    });

    finalPlacements_.clear();
    int rank = 1;
    for (auto& r : rows) {
        finalPlacements_.push_back({ r.c, rank++ });
    }
}

void MarbleManiaMap::Update(float dt) {
    // Static obstacles need no per-frame work
    world_.Step(dt, /*velocityIterations=*/10, /*positionIterations=*/8);
    simTime_ += dt;

    if (phase_ == GamePhase::SIMULATION) {
        // Finish line check (y increases downward)
        for (auto& kv : marbles_) {
            auto& m = *kv.second;
            if (!m.HasFinished() && m.GetPosition().y >= finishLineY_) {
                m.MarkFinished(simTime_);
            }
        }
        if (everyoneFinished_() || simTime_ >= maxSimTime_) {
            computeFinalPlacements_();
            phase_ = GamePhase::FINISHED;
        }
    }
}
