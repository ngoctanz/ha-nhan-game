#include "game/LoadingScreen.hpp"

#include "game/Config.hpp"
#include "game/TextureAsset.hpp"
#include "game/Ui.hpp"

#include <algorithm>
#include <cmath>

namespace game
{
LoadingScreen::~LoadingScreen()
{
    UnloadTextureAsset(dog_);
}

bool LoadingScreen::Load()
{
    dog_ = LoadTextureAsset("assets/characters/system_dog/idle.png");
    return dog_.id != 0;
}

void LoadingScreen::SetProgress(float progress)
{
    progress_ = std::clamp(progress, 0.0F, 1.0F);
}

void LoadingScreen::Update(float deltaTime)
{
    animationTime_ += std::max(0.0F, deltaTime);
}

void LoadingScreen::Draw(Font font) const
{
    const Color paper = {250, 250, 247, 255};
    const Color trackColor = {221, 226, 224, 255};
    const Color fillColor = {75, 190, 181, 255};
    ClearBackground(paper);

    if (dog_.id != 0)
    {
        const float height = 210.0F;
        const float width = height * static_cast<float>(dog_.width) /
                            static_cast<float>(dog_.height);
        const float bob = std::sin(animationTime_ * 2.8F) * 2.0F;
        DrawTexturePro(dog_, {0, 0, static_cast<float>(dog_.width),
                              static_cast<float>(dog_.height)},
                       {ScreenWidth / 2.0F - width / 2.0F, 210.0F + bob, width, height},
                       {0, 0}, 0.0F, WHITE);
    }

    const Rectangle track = {ScreenWidth / 2.0F - 260.0F, 474, 520, 14};
    DrawRectangleRounded(track, 0.5F, 8, trackColor);
    if (progress_ > 0.0F)
        DrawRectangleRounded({track.x, track.y, track.width * progress_, track.height},
                             0.5F, 8, fillColor);

    const int percentage = static_cast<int>(progress_ * 100.0F + 0.5F);
    const char *percentageText = TextFormat("%d%%", percentage);
    const Vector2 percentageSize = MeasureTextEx(font, percentageText, 18, 0.5F);
    DrawTextLine(font, percentageText,
                 {ScreenWidth / 2.0F - percentageSize.x / 2.0F, 508}, 18,
                 Color{92, 103, 104, 255});
}
} // namespace game
