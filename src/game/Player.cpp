#include "game/Player.hpp"
#include "game/Config.hpp"
#include "game/FaceRenderer.hpp"
#include "game/SpritePlacement.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace game
{
bool Player::Load()
{
    constexpr Rectangle animateStageCrop = {950.0F, 115.0F, 535.0F, 840.0F};
    const std::string root = "assets/characters/player/emotions/angry/actions/";
    // A single neutral frame keeps the silhouette perfectly still while idle.
    const std::vector<std::string> idlePaths = {root + "idle/idle_neutral.png"};
    std::vector<std::string> walkPaths;
    for (int i = 1; i <= 9; ++i)
        walkPaths.push_back(root + "walk/" + std::string(TextFormat("walk_%02d.png", i)));

    std::vector<std::string> sneakPaths;
    for (int i = 1; i <= 6; ++i)
        sneakPaths.push_back("assets/characters/player/actions/sneak/" +
                             std::string(TextFormat("sneak_%02d.png", i)));

    std::vector<std::string> goofyPaths;
    for (int i = 1; i <= 33; ++i)
        goofyPaths.push_back("assets/characters/player/actions/goofy_walk/" +
                             std::string(TextFormat("goofy_%02d.png", i)));

    const bool idleLoaded = idle_.Load(idlePaths, 1.0F, animateStageCrop, 267, 420);
    const bool walkLoaded = walk_.Load(walkPaths, 10.0F, animateStageCrop, 267, 420);
    const bool sneakLoaded = sneak_.Load(sneakPaths, 9.0F);
    const bool goofyLoaded = goofyWalk_.Load(goofyPaths, 22.0F);
    const bool actionsLoaded = actions_.Load(root + "gesture/", animateStageCrop, 267, 420);
    return idleLoaded && walkLoaded && sneakLoaded && goofyLoaded &&
           actionsLoaded;
}

void Player::TriggerAction(CharacterAction action)
{
    actions_.Trigger(action);
}

void Player::Update(float deltaTime, bool inputEnabled, const GameInput &input)
{
    if (actions_.IsActive())
    {
        moving_ = false;
        sprinting_ = false;
        motion_ = PlayerMotion::Action;
        return;
    }

    float direction = 0.0F;
    bool sprint = false;
    if (inputEnabled)
    {
        direction = input.moveAxis;
        sprint = input.sprintHeld;
    }

    moving_ = direction != 0.0F;
    sprinting_ = false;
    if (moving_)
    {
        facing_.Match(direction > 0.0F);
        const bool sneaking = input.sneakHeld;
        const bool beingGoofy = input.goofyHeld;
        float speed = sprint ? 430.0F : 270.0F;
        if (sneaking)
        {
            motion_ = PlayerMotion::Sneak;
            speed = 115.0F;
            sneak_.Update(deltaTime);
        }
        else if (beingGoofy)
        {
            motion_ = PlayerMotion::GoofyWalk;
            speed = 185.0F;
            goofyWalk_.Update(deltaTime);
        }
        else
        {
            motion_ = PlayerMotion::Walk;
            sprinting_ = sprint;
            walk_.Update(deltaTime * (sprint ? 1.35F : 1.0F));
        }
        position_.x = std::clamp(position_.x + direction * speed * deltaTime,
                                 95.0F, WorldWidth - 95.0F);
    }
    else
    {
        motion_ = PlayerMotion::Idle;
        idle_.Update(deltaTime);
    }
}

void Player::Draw(const Texture2D *face) const
{
    const Animation *animation = &idle_;
    if (motion_ == PlayerMotion::Walk) animation = &walk_;
    else if (motion_ == PlayerMotion::Sneak) animation = &sneak_;
    else if (motion_ == PlayerMotion::GoofyWalk) animation = &goofyWalk_;
    const Texture2D *texturePointer = nullptr;
    if (motion_ == PlayerMotion::Action) texturePointer = actions_.CurrentFrame();
    else if (animation->IsLoaded()) texturePointer = &animation->Current();
    if (texturePointer == nullptr || texturePointer->id == 0) return;
    const Texture2D &texture = *texturePointer;
    // The exported Hà Nhân artwork naturally faces left.
    const Rectangle source = SourceForFacing(texture, facing_, false);
    const bool normalizedAction = motion_ == PlayerMotion::Sneak || motion_ == PlayerMotion::GoofyWalk;
    float drawHeight = 335.0F;
    if (motion_ == PlayerMotion::Sneak) drawHeight = 258.0F;
    else if (motion_ == PlayerMotion::GoofyWalk) drawHeight = 292.0F;
    // Standard exports use a 372/420 foot line; normalized movement exports
    // use 340/360.  Both now land on the same world-space GroundY.
    const float normalizedFootY = normalizedAction ? 340.0F / 360.0F : 372.0F / 420.0F;
    const Rectangle destination =
        GroundedDestination(texture, position_, drawHeight, normalizedFootY);
    DrawGroundShadow(position_, destination.width * 0.32F, 9.0F, Fade(BLACK, 0.27F));
    DrawTexturePro(texture, source, destination, {0, 0}, 0.0F, WHITE);

    FaceAnchor faceAnchor;
    if (motion_ == PlayerMotion::Sneak)
        faceAnchor = {0.47F, 0.465F, 0.38F, 0.25F,
                      Color{250, 250, 247, 255}, 5.5F};
    else if (motion_ == PlayerMotion::GoofyWalk)
        faceAnchor = {0.42F, 0.43F, 0.40F, 0.29F, Color{250, 250, 247, 255}};
    else
        faceAnchor = {0.43F, 0.314F, 0.24F, 0.15F, Color{250, 250, 247, 255}};
    // The goofy walk was extracted from the supplied GIF and already contains
    // its intended meme face. Keep that artwork intact instead of stacking a
    // second FaceLibrary expression on top of it.
    if (motion_ != PlayerMotion::GoofyWalk)
        DrawFaceOverlay(face, destination, faceAnchor, facing_.IsFlipped(false));
}

Vector2 Player::Position() const
{
    return position_;
}

bool Player::FacingRight() const
{
    return facing_.IsRight();
}

bool Player::IsNear(float worldX, float distance) const
{
    return std::abs(position_.x - worldX) < distance;
}

const Texture2D *Player::PortraitTexture() const
{
    return idle_.IsLoaded() ? &idle_.Current() : nullptr;
}

const char *Player::SuggestedFaceExpression() const
{
    switch (motion_)
    {
        case PlayerMotion::Sneak: return "verified_playful_wink";
        case PlayerMotion::GoofyWalk: return "verified_curious_surprise";
        case PlayerMotion::Walk: return sprinting_ ? "verified_startled_panic" : "verified_displeased_smile";
        case PlayerMotion::Action: return actions_.SuggestedFace();
        case PlayerMotion::Idle: return "verified_roundface_teasing_smirk";
    }
    return "verified_roundface_teasing_smirk";
}
} // namespace game
