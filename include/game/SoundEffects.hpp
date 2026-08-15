#pragma once

#include <raylib.h>

#include <string_view>

namespace game
{
enum class MemeSound
{
    None,
    Huh,
    DoIt,
    CaoNiMa
};

[[nodiscard]] MemeSound ParseMemeSound(std::string_view name);

class SoundEffects
{
public:
    SoundEffects() = default;
    ~SoundEffects();
    SoundEffects(const SoundEffects &) = delete;
    SoundEffects &operator=(const SoundEffects &) = delete;

    bool Load();
    void Play(MemeSound sound);

private:
    Sound huh_ = {};
    Sound doIt_ = {};
    Sound caoNiMa_ = {};
};
} // namespace game
