#pragma once

#include "game/CharacterFacing.hpp"
#include "game/FaceLibrary.hpp"

#include <array>
#include <raylib.h>
#include <string_view>

namespace game
{
enum class RuffianEvent
{
    None,
    Retreated
};

class RuffianGroup
{
public:
    ~RuffianGroup();
    bool Load(const FaceLibrary &faces);
    void FaceToward(float targetX);
    void SetExpression(const FaceLibrary &faces, int index, std::string_view expression);
    void StartRetreat();
    RuffianEvent Update(float deltaTime, int talkingIndex = -1);
    void Draw(const FaceLibrary &faces, float worldTime) const;

    [[nodiscard]] const Texture2D *Portrait(int index) const;
    [[nodiscard]] const Texture2D *PortraitFace(const FaceLibrary &faces, int index) const;
    [[nodiscard]] Vector2 Position(int index) const;

private:
    static constexpr int Count = 4;
    std::array<Texture2D, Count> bodies_ = {};
    std::array<FaceAnimator, Count> faceAnimators_ = {};
    std::array<CharacterFacing, Count> facings_ = {
        CharacterFacing{FacingDirection::Right}, CharacterFacing{FacingDirection::Right},
        CharacterFacing{FacingDirection::Right}, CharacterFacing{FacingDirection::Right}};
    std::array<Vector2, Count> positions_ = {
        Vector2{1320, 622}, Vector2{1450, 622}, Vector2{1580, 622}, Vector2{1710, 622}};
    bool retreating_ = false;
};
} // namespace game
