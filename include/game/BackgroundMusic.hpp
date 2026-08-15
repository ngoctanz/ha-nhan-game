#pragma once

#include <raylib.h>

#include <string>

namespace game
{
class BackgroundMusic
{
public:
    BackgroundMusic() = default;
    ~BackgroundMusic();
    BackgroundMusic(const BackgroundMusic &) = delete;
    BackgroundMusic &operator=(const BackgroundMusic &) = delete;

    bool Load(const std::string &relativePath, float targetVolume);
    void Update(float deltaTime);

private:
    Music music_ = {};
    unsigned char *encodedData_ = nullptr;
    int encodedDataSize_ = 0;
    float targetVolume_ = 0.30F;
    float fadeVolume_ = 0.0F;
    bool loaded_ = false;
};
} // namespace game
