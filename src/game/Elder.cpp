#include "game/Elder.hpp"
#include "game/FaceRenderer.hpp"
#include "game/SpritePlacement.hpp"
#include "game/TextureAsset.hpp"

namespace game
{
Elder::~Elder()
{
    UnloadTextureAsset(fullBody_);
}

bool Elder::Load()
{
    const std::string root = "assets/characters/npc/elder/expressions/";
    fullBody_ = LoadTextureAsset(root + "fullbody_meme.png");
    return fullBody_.id != 0;
}

void Elder::FaceToward(float selfX, float targetX)
{
    facing_.FaceToward(selfX, targetX);
}

void Elder::Draw(Vector2 feet, float worldTime, const Texture2D *face) const
{
    if (fullBody_.id == 0) return;
    (void)worldTime;
    constexpr float height = 310.0F;
    // Alpha ends at y=1440 on the 1493 px source; the remaining transparent
    // padding must sit below the ground contact point rather than lift the feet.
    constexpr float normalizedFootY = 1440.0F / 1493.0F;
    const Rectangle destination =
        GroundedDestination(fullBody_, feet, height, normalizedFootY);
    DrawGroundShadow(feet, destination.width * 0.30F, 9.0F, Fade(BLACK, 0.24F));
    constexpr bool bodyArtworkFacesRight = false;
    // The wise-goatee face artwork also naturally leans left. Declaring it as
    // right-facing caused an unnecessary second flip and reversed the gaze.
    constexpr bool faceArtworkFacesRight = false;
    const bool bodyFlipped = facing_.IsFlipped(bodyArtworkFacesRight);
    const bool faceFlipped = facing_.IsFlipped(faceArtworkFacesRight);
    DrawTexturePro(fullBody_,
                   SourceForFacing(fullBody_, facing_, bodyArtworkFacesRight),
                   destination, {0, 0}, 0.0F, WHITE);
    const FaceAnchor elderFace = {
        0.605F, 0.395F, 0.50F, 0.22F,
        Color{250, 241, 224, 255}, 0.0F, false, -0.090F};
    DrawFaceOverlay(face, destination, elderFace, bodyFlipped, faceFlipped);
}

const Texture2D *Elder::Portrait(std::string_view emotion) const
{
    (void)emotion;
    return fullBody_.id == 0 ? nullptr : &fullBody_;
}
} // namespace game
