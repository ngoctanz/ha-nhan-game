#include "game/SplashScreen.hpp"
#include "game/AssetLocator.hpp"
#include "game/Config.hpp"

#include <algorithm>
#include <cmath>

namespace game
{
SplashScreen::~SplashScreen()
{
    if (logo_.id != 0) UnloadTexture(logo_);
}

bool SplashScreen::Load()
{
    const std::string path = ResolveAssetPath("assets/images/logo.png");
    logo_ = LoadTexture(path.c_str());
    if (logo_.id != 0) SetTextureFilter(logo_, TEXTURE_FILTER_BILINEAR);
    return logo_.id != 0;
}

bool SplashScreen::Update(float deltaTime)
{
    timer_ += deltaTime;
    return timer_ < Duration;
}

void SplashScreen::Draw(Font font) const
{
    // Alpha curve: ramp up → hold → ramp down
    float alpha = 0.0F;
    if (timer_ < FadeIn)
        alpha = timer_ / FadeIn;
    else if (timer_ < FadeIn + Hold)
        alpha = 1.0F;
    else
        alpha = 1.0F - (timer_ - FadeIn - Hold) / FadeOut;
    alpha = std::clamp(alpha, 0.0F, 1.0F);

    // Subtle pulse during the hold phase to feel alive
    const float pulse = (timer_ >= FadeIn && timer_ < FadeIn + Hold)
        ? 1.0F + std::sin((timer_ - FadeIn) * 3.0F) * 0.018F
        : 1.0F;

    ClearBackground(BLACK);

    // Logo — centered perfectly
    if (logo_.id != 0)
    {
        const float logoW = ScreenWidth * 0.32F * pulse;
        const float logoH = logoW * (static_cast<float>(logo_.height) /
                                     static_cast<float>(logo_.width));
        const float logoX = (ScreenWidth  - logoW) / 2.0F;
        const float logoY = (ScreenHeight - logoH) / 2.0F;
        DrawTexturePro(logo_,
            {0, 0, static_cast<float>(logo_.width), static_cast<float>(logo_.height)},
            {logoX, logoY, logoW, logoH},
            {0, 0}, 0.0F, Fade(WHITE, alpha));
    }
}
} // namespace game
