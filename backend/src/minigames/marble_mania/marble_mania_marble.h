#ifndef PEOPLEPARTY_BACKEND_MARBLE_MANIA_MARBLE_H
#define PEOPLEPARTY_BACKEND_MARBLE_MANIA_MARBLE_H

#include <string>
#include <box2d/box2d.h>
#include "../../util/math/vector2d.h"
#include "../../client.h"

class MarbleManiaMarble {
public:
    MarbleManiaMarble(b2World& world,
                      const std::string& id,
                      const Vector2D& position,
                      float radius);

    ~MarbleManiaMarble();

    // Placement-only helper
    void Teleport(const Vector2D& position);
    void EnableGravity(bool enable);

    // Accessors
    Vector2D GetPosition() const;  // Returns physics coordinates
    Vector2D GetWorldPosition() const;  // Returns world coordinates (scaled up)
    float GetRadius() const { return radius_; }
    float GetWorldRadius() const;  // Returns world radius (scaled up)
    bool HasFinished() const { return finished_; }
    void MarkFinished(float timeToFinishSeconds);
    float GetTimeToFinish() const { return timeToFinish_; }

    const std::string& GetId() const { return id_; }
    b2Body* GetBody() const { return body_; }

private:
    std::string id_;
    float radius_;
    b2Body* body_ = nullptr;

    bool finished_ = false;
    float timeToFinish_ = 0.0f;
};

#endif // PEOPLEPARTY_BACKEND_MARBLE_MANIA_MARBLE_H
