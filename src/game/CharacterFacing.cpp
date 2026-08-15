#include "game/CharacterFacing.hpp"

namespace game
{
CharacterFacing::CharacterFacing(FacingDirection initial) : direction_(initial) {}

void CharacterFacing::FaceToward(float selfX, float targetX, float deadZone)
{
    if (targetX > selfX + deadZone) direction_ = FacingDirection::Right;
    else if (targetX < selfX - deadZone) direction_ = FacingDirection::Left;
}

void CharacterFacing::Match(bool facingRight)
{
    direction_ = facingRight ? FacingDirection::Right : FacingDirection::Left;
}

void CharacterFacing::Set(FacingDirection direction)
{
    direction_ = direction;
}

FacingDirection CharacterFacing::Direction() const
{
    return direction_;
}

bool CharacterFacing::IsRight() const
{
    return direction_ == FacingDirection::Right;
}

bool CharacterFacing::IsFlipped(bool artworkFacesRight) const
{
    return IsRight() != artworkFacesRight;
}

Rectangle SourceForFacing(const Texture2D &texture, const CharacterFacing &facing,
                          bool artworkFacesRight)
{
    Rectangle source = {0.0F, 0.0F, static_cast<float>(texture.width),
                        static_cast<float>(texture.height)};
    if (facing.IsFlipped(artworkFacesRight)) source.width = -source.width;
    return source;
}
} // namespace game
