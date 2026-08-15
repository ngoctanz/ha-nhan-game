#include "game/Game.hpp"
#include "game/Config.hpp"

#include <algorithm>

namespace game
{
namespace
{
const char *MusicForProgress(const std::string &levelId, const std::string &checkpoint)
{
    if (levelId == "village_gate") return "assets/music/troll.mp3";
    if (levelId == "neighbor_night")
    {
        const bool stillEscaping = checkpoint == "return_village" ||
                                   checkpoint == "returning_to_neighbor";
        return stillEscaping ? "assets/music/troll.mp3" : "assets/music/map3.mp3";
    }
    return "assets/music/demo_village.mp3";
}

float MusicVolume(const std::string &track)
{
    if (track == "assets/music/troll.mp3") return 0.23F;
    if (track == "assets/music/map3.mp3") return 0.26F;
    return 0.30F;
}
} // namespace

bool Game::Load()
{
    uiLoaded_ = ui_.Load();
#if defined(PLATFORM_ANDROID) || defined(HA_NHAN_MOBILE_UI_PREVIEW)
    return uiLoaded_ && loadingScreen_.Load();
#else
    return uiLoaded_ && LoadGameplay();
#endif
}

bool Game::LoadGameplay()
{
    if (gameplayLoaded_) return true;
    if (!uiLoaded_ || !levels_.LoadInitial(LevelId::BedroomIntro)) return false;
    systemMenu_.Bind(levels_.Progress());
    // Music is optional: an unavailable audio device must not block gameplay.
    musicTrack_ = MusicForProgress(levels_.Progress().CurrentLevel(),
                                   levels_.Progress().Checkpoint());
    backgroundMusic_.Load(musicTrack_, MusicVolume(musicTrack_));
    gameplayLoaded_ = true;
    return true;
}

void Game::Run()
{
#if defined(PLATFORM_ANDROID)
    // Show studio splash before the first gameplay frame.
    // splash_.Load() returns false if the asset is missing → skip silently.
    if (splash_.Load())
    {
        while (!WindowShouldClose())
        {
            const float dt = std::min(GetFrameTime(), 1.0F / 20.0F);
            BeginDrawing();
            splash_.Draw(ui_.GetFont());
            EndDrawing();
            if (!splash_.Update(dt)) break;
        }
    }
#endif

#if defined(PLATFORM_ANDROID) || defined(HA_NHAN_MOBILE_UI_PREVIEW)
    loadingScreen_.SetProgress(0.16F);
    float loadingLeadTime = 0.0F;
    while (!WindowShouldClose() && loadingLeadTime < 0.18F)
    {
        const float dt = std::min(GetFrameTime(), 1.0F / 20.0F);
        loadingLeadTime += dt;
        loadingScreen_.Update(dt);
        BeginDrawing();
        loadingScreen_.Draw(ui_.GetFont());
        EndDrawing();
    }
    if (WindowShouldClose() || !LoadGameplay()) return;

    loadingScreen_.SetProgress(1.0F);
    float loadingCompleteTime = 0.0F;
    while (!WindowShouldClose() && loadingCompleteTime < 0.32F)
    {
        const float dt = std::min(GetFrameTime(), 1.0F / 20.0F);
        loadingCompleteTime += dt;
        loadingScreen_.Update(dt);
        backgroundMusic_.Update(dt);
        BeginDrawing();
        loadingScreen_.Draw(ui_.GetFont());
        EndDrawing();
    }

    float gameplayReveal = 0.0F;
    while (!WindowShouldClose() && gameplayReveal < 1.0F)
    {
        const float dt = std::min(GetFrameTime(), 1.0F / 20.0F);
        gameplayReveal = std::min(1.0F, gameplayReveal + dt / 0.90F);
        backgroundMusic_.Update(dt);
        BeginDrawing();
        ClearBackground(Color{22, 35, 46, 255});
        levels_.Draw(ui_);
        DrawRectangle(0, 0, ScreenWidth, ScreenHeight,
                      Fade(BLACK, 1.0F - gameplayReveal));
        EndDrawing();
    }
#endif

    while (!WindowShouldClose())
    {
        const float deltaTime = std::min(GetFrameTime(), 1.0F / 20.0F);
        backgroundMusic_.Update(deltaTime);
        const GameInput input = input_.Poll();
        systemMenu_.BindCombat(levels_.ActiveCombat());
        const bool menuConsumedInput = !levels_.IsTransitioning() &&
                                       systemMenu_.Update(input, deltaTime);
        if (!systemMenu_.IsOpen() && !menuConsumedInput)
            levels_.Update(deltaTime, input);
        if (levels_.ConsumeSystemMenuCommand() == SystemMenuCommand::PreviewStorage)
            systemMenu_.PreviewStorage(1.6F);
        const char *desiredMusic = MusicForProgress(levels_.Progress().CurrentLevel(),
                                                    levels_.Progress().Checkpoint());
        if (musicTrack_ != desiredMusic)
        {
            musicTrack_ = desiredMusic;
            backgroundMusic_.Load(musicTrack_, MusicVolume(musicTrack_));
        }
        BeginDrawing();
        ClearBackground(Color{22, 35, 46, 255});
        levels_.Draw(ui_);
        if (!levels_.IsTransitioning()) systemMenu_.Draw(ui_);
        EndDrawing();
    }
}
} // namespace game
