#pragma once

#include <raylib.h>

namespace game
{
// Full-screen studio splash shown once at startup (Android only).
// Timeline: fade-in (0.6 s) → hold (1.2 s) → fade-out (0.7 s) → done.
class SplashScreen
{
public:
    ~SplashScreen();

    // Load logo texture. Returns false if asset missing (splash is skipped).
    bool Load();

    // Advance the splash by deltaTime. Returns true while the splash is still
    // running; returns false once the fade-out finishes.
    bool Update(float deltaTime);

    // Draw the splash. Call only while Update() returns true.
    void Draw(Font font) const;

private:
    Texture2D logo_ = {};
    float timer_    = 0.0F;

    static constexpr float FadeIn   = 1.0F;
    static constexpr float Hold     = 2.5F;
    static constexpr float FadeOut  = 1.0F;
    static constexpr float Duration = FadeIn + Hold + FadeOut;
};
} // namespace game
