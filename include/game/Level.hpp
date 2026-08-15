#pragma once

namespace game
{
class Ui;
struct GameInput;

class Level
{
public:
    virtual ~Level() = default;
    virtual bool Load() = 0;
    virtual void Update(float deltaTime, const GameInput &input) = 0;
    virtual void Draw(const Ui &ui) const = 0;
};
} // namespace game
