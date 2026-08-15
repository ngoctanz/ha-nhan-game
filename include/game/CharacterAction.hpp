#pragma once

#include <raylib.h>

#include <array>
#include <string>
#include <string_view>

namespace game
{
enum class CharacterAction
{
    None,
    HandsFolded,
    Point,
    PresentLow,
    PresentOpen,
    Wave
};

[[nodiscard]] CharacterAction ParseCharacterAction(std::string_view name);

class CharacterActionPlayer
{
public:
    CharacterActionPlayer() = default;
    ~CharacterActionPlayer();
    CharacterActionPlayer(const CharacterActionPlayer &) = delete;
    CharacterActionPlayer &operator=(const CharacterActionPlayer &) = delete;

    bool Load(const std::string &assetRoot, Rectangle crop,
              int resizedWidth, int resizedHeight);
    void Trigger(CharacterAction action);
    void Clear();

    [[nodiscard]] bool IsActive() const;
    [[nodiscard]] const Texture2D *CurrentFrame() const;
    [[nodiscard]] const char *SuggestedFace() const;
    [[nodiscard]] int LoadedPoseCount() const;

private:
    enum PoseIndex
    {
        HandsFolded,
        Point,
        PresentLow,
        PresentOpen,
        Wave,
        PoseCount
    };

    std::array<Texture2D, PoseCount> poses_ = {};
    int activePose_ = HandsFolded;
    CharacterAction action_ = CharacterAction::None;
    bool active_ = false;
};
} // namespace game
