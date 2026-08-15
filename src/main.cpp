#include "game/Config.hpp"
#include "game/Game.hpp"

#include <raylib.h>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(game::ScreenWidth, game::ScreenHeight, "Hà Nhân Xuyên Không - Demo Thôn Trúc");
    InitAudioDevice();
    SetTargetFPS(game::TargetFps);

    int result = 0;
    {
        game::Game application;
        if (application.Load()) application.Run();
        else result = 1;
    }

    if (IsAudioDeviceReady()) CloseAudioDevice();
    CloseWindow();
    return result;
}
