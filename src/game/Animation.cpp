#include "game/Animation.hpp"
#include "game/TextureAsset.hpp"

#include <cmath>

namespace game
{
Animation::~Animation()
{
    for (Texture2D &texture : frames_) UnloadTextureAsset(texture);
}

bool Animation::Load(const std::vector<std::string> &paths, float fps,
                     Rectangle crop, int resizedWidth, int resizedHeight)
{
    fps_ = fps;
    for (const std::string &relative : paths)
    {
        Texture2D texture = LoadTextureAsset(relative, crop, resizedWidth, resizedHeight);
        if (texture.id == 0) continue;
        frames_.push_back(texture);
    }
    return !frames_.empty();
}

void Animation::Update(float deltaTime)
{
    if (frames_.empty()) return;
    frameCursor_ = std::fmod(frameCursor_ + fps_ * deltaTime, static_cast<float>(frames_.size()));
}

bool Animation::UpdateOnce(float deltaTime)
{
    if (frames_.empty()) return true;
    frameCursor_ += fps_ * deltaTime;
    if (frameCursor_ < static_cast<float>(frames_.size())) return false;
    frameCursor_ = static_cast<float>(frames_.size() - 1);
    return true;
}

void Animation::Reset()
{
    frameCursor_ = 0.0F;
}

bool Animation::IsLoaded() const
{
    return !frames_.empty();
}

const Texture2D &Animation::Current() const
{
    return frames_[static_cast<int>(frameCursor_) % frames_.size()];
}

int Animation::FrameCount() const
{
    return static_cast<int>(frames_.size());
}

int Animation::CurrentIndex() const
{
    if (frames_.empty()) return 0;
    return static_cast<int>(frameCursor_) % static_cast<int>(frames_.size());
}
} // namespace game
