#include "marble_mania_marble.h"
#include <sstream>

MarbleManiaMarble::MarbleManiaMarble(PhysicsObject* physicsObject, Client* owner, float radius)
    : m_physicsObject(physicsObject), m_owner(owner), m_radius(radius)
    , m_hasFinished(false), m_timeToFinish(0.0f), m_placement(0), m_isPlaced(false)
{
    // Generate unique ID for this marble
    std::stringstream ss;
    ss << "marble_" << owner->client_id;
    m_id = ss.str();
}

void MarbleManiaMarble::ResetToPosition(const Vector2D& position) {
    m_physicsObject->SetPosition(position);
    m_physicsObject->SetVelocity(Vector2D(0, 0));
    m_physicsObject->SetAcceleration(Vector2D(0, 0));
    
    // Reset game state
    m_hasFinished = false;
    m_timeToFinish = 0.0f;
    m_placement = 0;
}

void MarbleManiaMarble::Stop() {
    m_physicsObject->SetVelocity(Vector2D(0, 0));
    m_physicsObject->SetAcceleration(Vector2D(0, 0));
}
