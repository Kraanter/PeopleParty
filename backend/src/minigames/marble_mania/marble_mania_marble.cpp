#include "marble_mania_marble.h"
#include <sstream>

MarbleManiaMarble::MarbleManiaMarble(const Vector2D& position, Client* owner, MarbleType type)
    : MarbleBase(position, type)
    , m_owner(owner)
    , m_timeToFinish(0.0f)
    , m_hasFinished(false)
    , m_placement(0)
{
    // Generate unique ID for this marble
    if (owner) {
        std::stringstream ss;
        ss << "marble_" << owner->client_id << "_" << (int)position.x << "_" << (int)position.y;
        m_id = ss.str();
    } else {
        m_id = "marble_unknown";
    }
}

void MarbleManiaMarble::CheckFinishLine(float finishY) {
    if (!m_hasFinished && GetPosition().y >= finishY) {
        m_hasFinished = true;
        // Stop the marble when it crosses the finish line
        SetVelocity(Vector2D(0, 0));
        SetStatic(true);
    }
}

void MarbleManiaMarble::UpdateTimer(float deltaTime) {
    if (!m_hasFinished) {
        m_timeToFinish += deltaTime;
    }
}

MarbleManiaMarble* MarbleManiaMarble::CreatePlayerMarble(const Vector2D& position, Client* owner) {
    return new MarbleManiaMarble(position, owner, MarbleType::NORMAL);
}
