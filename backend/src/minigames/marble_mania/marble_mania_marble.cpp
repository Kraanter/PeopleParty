#include "marble_mania_marble.h"

static b2Vec2 toB2(const Vector2D& v){ return b2Vec2(v.x, v.y); }
static Vector2D toV2(const b2Vec2& v){ return Vector2D{v.x, v.y}; }

MarbleManiaMarble::MarbleManiaMarble(b2World& world,
                                     const std::string& id,
                                     const Vector2D& position,
                                     float radius)
: id_(id), radius_(radius)
{
    b2BodyDef bd;
    bd.type = b2_dynamicBody;
    bd.position = toB2(position);
    body_ = world.CreateBody(&bd);
    body_->SetBullet(true); // better CCD vs edges/contacts

    b2CircleShape shape;
    shape.m_radius = radius_;

    b2FixtureDef fd;
    fd.shape = &shape;
    fd.density = 1.0f;
    fd.friction = 0.2f;
    fd.restitution = 0.8f; // bouncy
    body_->CreateFixture(&fd);

    // During placement, keep gravity off
    body_->SetGravityScale(0.0f);
    body_->SetLinearVelocity(b2Vec2_zero);
    body_->SetAngularVelocity(0.0f);
}

MarbleManiaMarble::~MarbleManiaMarble() {
    if (body_ && body_->GetWorld()) {
        body_->GetWorld()->DestroyBody(body_);
    }
}

void MarbleManiaMarble::Teleport(const Vector2D& position) {
    if (!body_) return;
    body_->SetLinearVelocity(b2Vec2_zero);
    body_->SetAngularVelocity(0.0f);
    body_->SetTransform(toB2(position), 0.0f);
}

void MarbleManiaMarble::EnableGravity(bool enable) {
    if (body_) body_->SetGravityScale(enable ? 1.0f : 0.0f);
}

Vector2D MarbleManiaMarble::GetPosition() const {
    return toV2(body_->GetPosition());
}

void MarbleManiaMarble::MarkFinished(float t) {
    finished_ = true;
    timeToFinish_ = t;
    if (body_) {
        body_->SetEnabled(false); // invisible/static for gameplay purposes
    }
}
