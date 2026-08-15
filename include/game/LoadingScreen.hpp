#pragma once

#include <raylib.h>

namespace game
{
class LoadingScreen
{
public:
    ~LoadingScreen();
    bool Load();
    void SetProgress(float progress);
    void Update(float deltaTime);
    void Draw(Font font) const;

private:
    float progress_ = 0.0F;
    float animationTime_ = 0.0F;
    Texture2D dog_ = {};
};
} // namespace game
