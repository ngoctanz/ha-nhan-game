#pragma once

#include <raylib.h>

namespace game
{
enum class FacingDirection
{
    Left,
    Right
};

class CharacterFacing
{
public:
    explicit CharacterFacing(FacingDirection initial = FacingDirection::Right);

    void FaceToward(float selfX, float targetX, float deadZone = 4.0F);
    void Match(bool facingRight);
    void Set(FacingDirection direction);

    [[nodiscard]] FacingDirection Direction() const;
    [[nodiscard]] bool IsRight() const;
    [[nodiscard]] bool IsFlipped(bool artworkFacesRight = true) const;

private:
    FacingDirection direction_ = FacingDirection::Right;
};

[[nodiscard]] Rectangle SourceForFacing(const Texture2D &texture,
                                        const CharacterFacing &facing,
                                        bool artworkFacesRight = true);
} // namespace game
