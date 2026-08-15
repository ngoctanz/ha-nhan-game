#pragma once

#include "game/BackgroundMusic.hpp"
#include "game/GameInput.hpp"
#include "game/LevelManager.hpp"
#include "game/LoadingScreen.hpp"
#include "game/SplashScreen.hpp"
#include "game/SystemMenu.hpp"
#include "game/Ui.hpp"

#include <string>

namespace game
{
class Game
{
public:
    bool Load();
    void Run();

private:
    bool LoadGameplay();

    BackgroundMusic backgroundMusic_;
    std::string musicTrack_;
    InputSystem input_;
    Ui ui_;
    LevelManager levels_;
    SystemMenu systemMenu_;
    SplashScreen splash_;
    LoadingScreen loadingScreen_;
    bool uiLoaded_ = false;
    bool gameplayLoaded_ = false;
};
} // namespace game
