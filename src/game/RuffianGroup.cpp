#include "game/RuffianGroup.hpp"

#include "game/FaceRenderer.hpp"
#include "game/SpritePlacement.hpp"
#include "game/TextureAsset.hpp"

#include <algorithm>
#include <cmath>

namespace game
{
RuffianGroup::~RuffianGroup()
{
    for (Texture2D &body : bodies_) UnloadTextureAsset(body);
}

bool RuffianGroup::Load(const FaceLibrary &faces)
{
    const std::array<const char *, Count> paths = {
        "assets/characters/bandits/poor_farmer/idle.png",
        "assets/characters/bandits/woodcutter/idle.png",
        "assets/characters/bandits/hunched_drifter/idle.png",
        "assets/characters/bandits/grain_porter/idle.png"};
    bool loaded = true;
    for (int i = 0; i < Count; ++i)
    {
        bodies_[i] = LoadTextureAsset(paths[i]);
        loaded = loaded && bodies_[i].id != 0;
    }
    const std::array<std::string_view, Count> expressions = {
        "devious", "knowing_smirk", "smug", "laughing"};
    for (int i = 0; i < Count; ++i)
        faceAnimators_[i].SetExpression(faces, FaceGender::Male, expressions[i]);
    return loaded;
}

void RuffianGroup::FaceToward(float targetX)
{
    for (int i = 0; i < Count; ++i)
        facings_[i].FaceToward(positions_[i].x, targetX);
}

void RuffianGroup::SetExpression(const FaceLibrary &faces, int index,
                                 std::string_view expression)
{
    if (index < 0 || index >= Count) return;
    faceAnimators_[index].SetExpression(faces, FaceGender::Male, expression);
}

void RuffianGroup::StartRetreat()
{
    retreating_ = true;
    for (CharacterFacing &facing : facings_) facing.Set(FacingDirection::Right);
}

RuffianEvent RuffianGroup::Update(float deltaTime, int talkingIndex)
{
    if (talkingIndex >= 0 && talkingIndex < Count)
        faceAnimators_[talkingIndex].Update(deltaTime);
    if (!retreating_) return RuffianEvent::None;
    for (Vector2 &position : positions_) position.x += 310.0F * deltaTime;
    if (positions_.front().x > 2550.0F)
    {
        retreating_ = false;
        return RuffianEvent::Retreated;
    }
    return RuffianEvent::None;
}

void RuffianGroup::Draw(const FaceLibrary &faces, float worldTime) const
{
    // The first two bodies were authored looking right; the drifter and porter
    // were authored looking left. Shared male FaceLibrary artwork looks left.
    // Keep those directions independent so every actor turns as one group.
    static constexpr std::array<bool, Count> bodyArtworkFacesRight = {
        true, true, false, false};
    constexpr bool faceArtworkFacesRight = false;
    static constexpr std::array<FaceAnchor, Count> anchors = {{
        {0.519F, 0.300F, 0.19F, 0.15F, Color{250, 238, 220, 255}, 0.0F, false},
        {0.506F, 0.318F, 0.20F, 0.15F, Color{250, 238, 220, 255}, 0.0F, false},
        {0.428F, 0.246F, 0.14F, 0.15F, Color{250, 238, 220, 255}, 0.0F, false},
        {0.495F, 0.280F, 0.20F, 0.16F, Color{250, 238, 220, 255}, 0.0F, false}
    }};
    for (int i = 0; i < Count; ++i)
    {
        if (bodies_[i].id == 0) continue;
        Vector2 feet = positions_[i];
        if (retreating_) feet.y += std::sin(worldTime * 17.0F + i) * 4.0F;
        const Rectangle destination = GroundedDestination(
            bodies_[i], feet, 270.0F, 488.0F / 512.0F);
        DrawGroundShadow(feet, destination.width * 0.28F, 8.0F, Fade(BLACK, 0.24F));
        DrawTexturePro(bodies_[i],
                       SourceForFacing(bodies_[i], facings_[i], bodyArtworkFacesRight[i]),
                       destination, {0, 0}, 0.0F, WHITE);
        const bool bodyFlipped = facings_[i].IsFlipped(bodyArtworkFacesRight[i]);
        const bool faceFlipped = facings_[i].IsFlipped(faceArtworkFacesRight);
        DrawFaceOverlay(faceAnimators_[i].Current(faces), destination, anchors[i],
                        bodyFlipped, faceFlipped);
    }
}

const Texture2D *RuffianGroup::Portrait(int index) const
{
    return index >= 0 && index < Count && bodies_[index].id != 0 ? &bodies_[index] : nullptr;
}

const Texture2D *RuffianGroup::PortraitFace(const FaceLibrary &faces, int index) const
{
    return index >= 0 && index < Count ? faceAnimators_[index].Current(faces) : nullptr;
}

Vector2 RuffianGroup::Position(int index) const
{
    return index >= 0 && index < Count ? positions_[index] : Vector2{};
}
} // namespace game
