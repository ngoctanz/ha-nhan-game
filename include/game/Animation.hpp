#pragma once

#include <raylib.h>

#include <string>
#include <vector>

namespace game
{
class Animation
{
public:
    Animation() = default;
    ~Animation();
    Animation(const Animation &) = delete;
    Animation &operator=(const Animation &) = delete;

    bool Load(const std::vector<std::string> &paths, float fps,
              Rectangle crop = {}, int resizedWidth = 0, int resizedHeight = 0);
    void Update(float deltaTime);
    void Reset();
    [[nodiscard]] bool IsLoaded() const;
    [[nodiscard]] const Texture2D &Current() const;
    [[nodiscard]] int FrameCount() const;

private:
    std::vector<Texture2D> frames_;
    float frameCursor_ = 0.0F;
    float fps_ = 8.0F;
};
} // namespace game
