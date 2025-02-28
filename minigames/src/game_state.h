#ifndef INCLUDE_GPT_GAMESTATE_H_
#define INCLUDE_GPT_GAMESTATE_H_

#include <string>
class GameState {
public:
    virtual std::string Serialize() const = 0;
    virtual void Deserialize(const std::string& data) = 0;
    virtual ~GameState() {}
};

#endif  // INCLUDE_GPT_GAMESTATE_H_
