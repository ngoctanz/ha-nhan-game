#pragma once

#include "game/BackgroundMusic.hpp"
#include "game/GameInput.hpp"
#include "game/LevelManager.hpp"
#include "game/Ui.hpp"

namespace game
{
class Game
{
public:
    bool Load();
    void Run();

private:
    BackgroundMusic backgroundMusic_;
    InputSystem input_;
    Ui ui_;
    LevelManager levels_;
};
} // namespace game
