#pragma once

#include "game/Level.hpp"

#include <memory>

namespace game
{
class Ui;
struct GameInput;

enum class LevelId
{
    BambooVillage
};

class LevelManager
{
public:
    bool Load(LevelId id);
    void Update(float deltaTime, const GameInput &input);
    void Draw(const Ui &ui) const;

private:
    std::unique_ptr<Level> current_;
};
} // namespace game
