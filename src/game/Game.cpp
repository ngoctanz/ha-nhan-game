#include "game/Game.hpp"
#include "game/Config.hpp"

#include <algorithm>

namespace game
{
bool Game::Load()
{
    const bool gameLoaded = ui_.Load() && levels_.Load(LevelId::BambooVillage);
    // Music is optional: an unavailable audio device must not block gameplay.
    backgroundMusic_.Load("assets/music/demo_village.mp3", 0.30F);
    return gameLoaded;
}

void Game::Run()
{
    while (!WindowShouldClose())
    {
        const float deltaTime = std::min(GetFrameTime(), 1.0F / 20.0F);
        backgroundMusic_.Update(deltaTime);
        const GameInput input = input_.Poll();
        levels_.Update(deltaTime, input);
        BeginDrawing();
        ClearBackground(Color{22, 35, 46, 255});
        levels_.Draw(ui_);
        EndDrawing();
    }
}
} // namespace game
