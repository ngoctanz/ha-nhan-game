#include "game/SystemDog.hpp"
#include "game/TextureAsset.hpp"

#include <algorithm>
#include <cmath>

namespace game
{
namespace
{
constexpr float FormationOffsetX = 112.0F;
constexpr float SideSwitchDistance = 90.0F;

Texture2D LoadDogTexture(const char *name)
{
    return LoadTextureAsset(std::string("assets/characters/system_dog/") + name);
}
} // namespace

SystemDog::~SystemDog()
{
    UnloadTextureAsset(idle_);
    UnloadTextureAsset(quest_);
    UnloadTextureAsset(laugh_);
}

bool SystemDog::Load()
{
    idle_ = LoadDogTexture("idle.png");
    quest_ = LoadDogTexture("quest.png");
    laugh_ = LoadDogTexture("laugh.png");
    return idle_.id != 0 && quest_.id != 0 && laugh_.id != 0;
}

void SystemDog::Update(float deltaTime, Vector2 playerFeet, bool playerFacingRight)
{
    if (!positioned_)
    {
        formationSide_ = playerFacingRight ? -1 : 1;
        pendingSide_ = formationSide_;
        facing_.Match(playerFacingRight);
        lastPlayerX_ = playerFeet.x;
        position_ = {
            playerFeet.x + FormationOffsetX * formationSide_, playerFeet.y - 152.0F};
        positioned_ = true;
        return;
    }

    const float movement = playerFeet.x - lastPlayerX_;
    lastPlayerX_ = playerFeet.x;
    if (std::abs(movement) > 0.01F)
    {
        const bool movingRight = movement > 0.0F;
        const int desiredSide = movingRight ? -1 : 1;
        if (desiredSide == formationSide_)
        {
            pendingSide_ = formationSide_;
            pendingTravel_ = 0.0F;
            facing_.Match(movingRight);
        }
        else
        {
            if (pendingSide_ != desiredSide)
            {
                pendingSide_ = desiredSide;
                pendingTravel_ = 0.0F;
            }
            pendingTravel_ += std::abs(movement);
            if (pendingTravel_ >= SideSwitchDistance)
            {
                formationSide_ = desiredSide;
                pendingTravel_ = 0.0F;
                facing_.Match(movingRight);
            }
        }
    }

    const Vector2 target = {
        playerFeet.x + FormationOffsetX * formationSide_, playerFeet.y - 152.0F};
    const float follow = std::min(1.0F, deltaTime * 10.0F);
    position_.x += (target.x - position_.x) * follow;
    position_.y += (target.y - position_.y) * follow;
}

void SystemDog::DrawWorld(SystemDogPose pose, float time) const
{
    const Texture2D *texture = Pose(pose);
    if (!positioned_ || texture == nullptr || texture->id == 0) return;

    const float hover = std::sin(time * 3.2F) * 3.0F;
    constexpr float height = 112.0F;
    const float width = height * texture->width / static_cast<float>(texture->height);
    DrawEllipse(static_cast<int>(position_.x), static_cast<int>(position_.y + 18.0F),
                width * 0.30F, 6.0F, Fade(BLACK, 0.18F));
    const Rectangle source = SourceForFacing(*texture, facing_);
    const Rectangle destination = {position_.x - width / 2.0F,
                                   position_.y - height + hover,
                                   width, height};
    DrawTexturePro(*texture, source, destination, {0.0F, 0.0F}, 0.0F, WHITE);
}

const Texture2D *SystemDog::Pose(SystemDogPose pose) const
{
    switch (pose)
    {
        case SystemDogPose::Quest: return quest_.id != 0 ? &quest_ : nullptr;
        case SystemDogPose::Laugh: return laugh_.id != 0 ? &laugh_ : nullptr;
        case SystemDogPose::Idle: return idle_.id != 0 ? &idle_ : nullptr;
    }
    return nullptr;
}
} // namespace game
