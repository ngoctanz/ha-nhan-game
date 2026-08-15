#include "game/NeighborWoman.hpp"
#include "game/FaceRenderer.hpp"
#include "game/SpritePlacement.hpp"
#include "game/TextureAsset.hpp"

#include <cmath>

namespace game
{
NeighborWoman::~NeighborWoman()
{
    UnloadTextureAsset(body_);
}

bool NeighborWoman::Load()
{
    body_ = LoadTextureAsset("assets/characters/neighbor_woman/idle.png");
    return body_.id != 0;
}

void NeighborWoman::Place(Vector2 feet, float speakerX)
{
    position_ = feet;
    facing_.FaceToward(position_.x, speakerX);
}

void NeighborWoman::FaceToward(float speakerX)
{
    facing_.FaceToward(position_.x, speakerX);
}

void NeighborWoman::StartWalkTo(float targetX)
{
    walkTargetX_ = targetX;
    facing_.FaceToward(position_.x, targetX);
    walking_ = true;
    walkTime_ = 0.0F;
}

NeighborWomanEvent NeighborWoman::Update(float deltaTime)
{
    if (!walking_) return NeighborWomanEvent::None;
    walkTime_ += deltaTime;
    const float direction = walkTargetX_ >= position_.x ? 1.0F : -1.0F;
    const float nextX = position_.x + direction * 145.0F * deltaTime;
    const bool arrived = direction > 0.0F ? nextX >= walkTargetX_ : nextX <= walkTargetX_;
    position_.x = arrived ? walkTargetX_ : nextX;
    if (!arrived) return NeighborWomanEvent::None;
    walking_ = false;
    return NeighborWomanEvent::MoveFinished;
}

void NeighborWoman::Draw(const Texture2D *face) const
{
    if (body_.id == 0) return;
    constexpr float drawHeight = 335.0F;
    constexpr float normalizedFootY = 400.0F / 420.0F;
    Vector2 feet = position_;
    if (walking_) feet.y += std::sin(walkTime_ * 11.0F) * 3.0F;
    const Rectangle destination =
        GroundedDestination(body_, feet, drawHeight, normalizedFootY);
    DrawGroundShadow(position_, destination.width * 0.29F, 8.0F, Fade(BLACK, 0.22F));
    constexpr bool bodyArtworkFacesRight = true;
    // The generated body naturally faces right, while the shared female face
    // artwork is authored toward the opposite side. They must mirror independently.
    constexpr bool faceArtworkFacesRight = false;
    DrawTexturePro(body_, SourceForFacing(body_, facing_, bodyArtworkFacesRight),
                   destination, {0.0F, 0.0F}, 0.0F, WHITE);
    const FaceAnchor anchor = {
        0.555F, 0.314F, 0.22F, 0.16F,
        Color{250, 250, 247, 255}, 0.0F, false, 0.025F};
    DrawFaceOverlay(face, destination, anchor,
                    facing_.IsFlipped(bodyArtworkFacesRight),
                    facing_.IsFlipped(faceArtworkFacesRight));
}

const Texture2D *NeighborWoman::PortraitTexture() const
{
    return body_.id != 0 ? &body_ : nullptr;
}

Vector2 NeighborWoman::Position() const
{
    return position_;
}
} // namespace game
