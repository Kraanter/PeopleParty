#include "marble_mania_obstacle.h"

static b2Vec2 toB2(const Vector2D& v){ return b2Vec2(v.x, v.y); }
static Vector2D toV2(const b2Vec2& v){ return Vector2D{v.x, v.y}; }

MarbleManiaObstacle::MarbleManiaObstacle(b2World& world,
                                         const std::string& id,
                                         ObstacleType type,
                                         const Vector2D& position,
                                         float width, float height,
                                         float initialRotationRadians)
: id_(id), type_(type), width_(width), height_(height)
{
    createBodyAndFixture_(world, position, initialRotationRadians);
}

MarbleManiaObstacle::~MarbleManiaObstacle() {
    if (body_ && body_->GetWorld()) {
        body_->GetWorld()->DestroyBody(body_);
    }
}

void MarbleManiaObstacle::createBodyAndFixture_(b2World& world,
                                                const Vector2D& pos,
                                                float rotationRadians)
{
    b2BodyDef bd;
    bd.type = b2_staticBody;          // static, no motion
    bd.position = toB2(pos);
    bd.angle = rotationRadians;
    body_ = world.CreateBody(&bd);

    b2FixtureDef fd;
    fd.friction = 0.4f;
    fd.restitution = 0.6f;
    fd.density = 0.0f; // ignored by static, but explicit

    if (type_ == ObstacleType::Circle) {
        b2CircleShape circle;
        circle.m_radius = width_ * 0.5f; // width==height==diameter
        fd.shape = &circle;
        body_->CreateFixture(&fd);
    } else if (type_ == ObstacleType::Rectangle) {
        b2PolygonShape poly;
        poly.SetAsBox(width_ * 0.5f, height_ * 0.5f);
        fd.shape = &poly;
        body_->CreateFixture(&fd);
    } else { // Triangle
        const float hw = width_ * 0.5f;
        const float hh = height_ * 0.5f;
        b2Vec2 verts[3] = { {-hw, +hh}, {0.f, -hh}, {+hw, +hh} };
        b2PolygonShape tri;
        tri.Set(verts, 3);
        fd.shape = &tri;
        body_->CreateFixture(&fd);
    }
}

Vector2D MarbleManiaObstacle::GetPosition() const {
    return toV2(body_->GetPosition());
}

float MarbleManiaObstacle::GetCurrentRotation() const {
    return body_->GetAngle();
}

std::vector<Vector2D> MarbleManiaObstacle::GetTriangleLocalVerts() const {
    if (type_ != ObstacleType::Triangle) return {};
    const float hw = width_ * 0.5f;
    const float hh = height_ * 0.5f;
    return { {-hw, +hh}, {0.f, -hh}, {+hw, +hh} }; // local (pre-rotation) verts
}
