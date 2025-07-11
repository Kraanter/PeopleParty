#ifndef MARBLE_MANIA_MARBLE_H
#define MARBLE_MANIA_MARBLE_H

#include "../../util/physics/marble.h"
#include "../../client.h"
#include <string>

class MarbleManiaMarble : public MarbleBase {
private:
    Client* m_owner;
    std::string m_id;
    float m_timeToFinish;
    bool m_hasFinished;
    int m_placement;
    
public:
    MarbleManiaMarble(const Vector2D& position, Client* owner, MarbleType type = MarbleType::NORMAL);
    
    // Owner management
    void SetOwner(Client* owner) { m_owner = owner; }
    Client* GetOwner() const { return m_owner; }
    
    // ID management
    void SetId(const std::string& id) { m_id = id; }
    const std::string& GetId() const { return m_id; }
    
    // Timing and placement
    void SetTimeToFinish(float time) { m_timeToFinish = time; }
    float GetTimeToFinish() const { return m_timeToFinish; }
    
    void SetFinished(bool finished) { m_hasFinished = finished; }
    bool HasFinished() const { return m_hasFinished; }
    
    void SetPlacement(int placement) { m_placement = placement; }
    int GetPlacement() const { return m_placement; }
    
    // Game-specific methods
    void CheckFinishLine(float finishY);
    void UpdateTimer(float deltaTime);
    
    // Create factory method for game marble
    static MarbleManiaMarble* CreatePlayerMarble(const Vector2D& position, Client* owner);
};

#endif // MARBLE_MANIA_MARBLE_H
