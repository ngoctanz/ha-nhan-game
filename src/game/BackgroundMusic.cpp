#include "game/BackgroundMusic.hpp"
#include "game/AssetLocator.hpp"

#include <algorithm>

namespace game
{
BackgroundMusic::~BackgroundMusic()
{
    if (loaded_)
    {
        StopMusicStream(music_);
        UnloadMusicStream(music_);
    }
    if (encodedData_ != nullptr) UnloadFileData(encodedData_);
}

bool BackgroundMusic::Load(const std::string &relativePath, float targetVolume)
{
    if (!IsAudioDeviceReady()) return false;

    const std::string path = ResolveAssetPath(relativePath);
    encodedData_ = LoadFileData(path.c_str(), &encodedDataSize_);
    if (encodedData_ == nullptr || encodedDataSize_ <= 0) return false;

    music_ = LoadMusicStreamFromMemory(".mp3", encodedData_, encodedDataSize_);
    loaded_ = IsAudioStreamValid(music_.stream);
    if (!loaded_) return false;

    targetVolume_ = std::clamp(targetVolume, 0.0F, 1.0F);
    fadeVolume_ = 0.0F;
    SetMusicVolume(music_, fadeVolume_);
    PlayMusicStream(music_);
    return true;
}

void BackgroundMusic::Update(float deltaTime)
{
    if (!loaded_) return;
    UpdateMusicStream(music_);
    if (!IsMusicStreamPlaying(music_)) PlayMusicStream(music_);

    constexpr float FadeInSeconds = 1.8F;
    fadeVolume_ = std::min(targetVolume_,
                           fadeVolume_ + targetVolume_ * deltaTime / FadeInSeconds);
    SetMusicVolume(music_, fadeVolume_);
}
} // namespace game
