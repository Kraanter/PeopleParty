#include "marble_mania_marble.h"
#include <sstream>

MarbleManiaMarble::MarbleManiaMarble(b2Body* body, Client* owner, float radius)
    : m_body(body), m_owner(owner), m_radius(radius)
    , m_hasFinished(false), m_timeToFinish(0.0f), m_placement(0), m_isPlaced(false)
{
    // Generate unique ID for this marble
    std::stringstream ss;
    ss << "marble_" << owner->client_id;
    m_id = ss.str();
}

void MarbleManiaMarble::SetRestitution(float restitution) {
    b2Fixture* fixture = m_body->GetFixtureList();
    while (fixture) {
        fixture->SetRestitution(restitution);
        fixture = fixture->GetNext();
    }
}

void MarbleManiaMarble::SetFriction(float friction) {
    b2Fixture* fixture = m_body->GetFixtureList();
    while (fixture) {
        fixture->SetFriction(friction);
        fixture = fixture->GetNext();
    }
}

void MarbleManiaMarble::SetDensity(float density) {
    b2Fixture* fixture = m_body->GetFixtureList();
    while (fixture) {
        fixture->SetDensity(density);
        fixture = fixture->GetNext();
    }
    m_body->ResetMassData();
}

void MarbleManiaMarble::ResetToPosition(const Vector2D& position) {
    m_body->SetTransform(b2Vec2(position.x, position.y), 0.0f);
    m_body->SetLinearVelocity(b2Vec2(0, 0));
    m_body->SetAngularVelocity(0.0f);
    
    // Reset game state
    m_hasFinished = false;
    m_timeToFinish = 0.0f;
    m_placement = 0;
}

void MarbleManiaMarble::Stop() {
    m_body->SetLinearVelocity(b2Vec2(0, 0));
    m_body->SetAngularVelocity(0.0f);
}
