#include "game/Chicken.hpp"
#include "game/SpritePlacement.hpp"
#include "game/TextureAsset.hpp"

#include <algorithm>
#include <string>

namespace game
{
namespace
{
Texture2D LoadChickenTexture(const char *name)
{
    return LoadTextureAsset(std::string("assets/characters/creatures/chicken/") + name);
}
} // namespace

Chicken::~Chicken()
{
    UnloadTextureAsset(idle_);
    UnloadTextureAsset(suspicious_);
    UnloadTextureAsset(run_);
    UnloadTextureAsset(startled_);
}

bool Chicken::Load()
{
    idle_ = LoadChickenTexture("idle.png");
    suspicious_ = LoadChickenTexture("suspicious.png");
    run_ = LoadChickenTexture("run.png");
    startled_ = LoadChickenTexture("startled.png");
    return idle_.id != 0 && suspicious_.id != 0 &&
           run_.id != 0 && startled_.id != 0;
}

void Chicken::SetPosition(Vector2 feet)
{
    position_ = feet;
    runOrigin_ = feet;
    runTarget_ = feet;
}

void Chicken::SetAlert(bool alert)
{
    alertAfterAction_ = alert;
    if (state_ == ChickenState::Idle || state_ == ChickenState::Alert)
        state_ = alert ? ChickenState::Alert : ChickenState::Idle;
}

void Chicken::StartEscape(Vector2 destination, bool vanishAfterRun)
{
    if (!CanInteract()) return;
    runOrigin_ = position_;
    runTarget_ = destination;
    facing_.FaceToward(runOrigin_.x, runTarget_.x, 0.0F);
    vanishAfterRun_ = vanishAfterRun;
    stateTimer_ = 0.0F;
    state_ = ChickenState::Startled;
}

ChickenEvent Chicken::Update(float deltaTime)
{
    stateTimer_ += deltaTime;
    switch (state_)
    {
        case ChickenState::Startled:
            if (stateTimer_ >= 0.22F)
            {
                stateTimer_ = 0.0F;
                state_ = ChickenState::Running;
            }
            break;
        case ChickenState::Running:
        {
            constexpr float runDuration = 0.62F;
            const float progress = std::min(1.0F, stateTimer_ / runDuration);
            const float smooth = progress * progress * (3.0F - 2.0F * progress);
            position_.x = runOrigin_.x + (runTarget_.x - runOrigin_.x) * smooth;
            position_.y = runOrigin_.y + (runTarget_.y - runOrigin_.y) * smooth;
            if (progress >= 1.0F)
            {
                position_ = runTarget_;
                stateTimer_ = 0.0F;
                if (vanishAfterRun_)
                {
                    state_ = ChickenState::Hidden;
                    return ChickenEvent::Vanished;
                }
                state_ = ChickenState::Settling;
            }
            break;
        }
        case ChickenState::Settling:
            if (stateTimer_ >= 0.32F)
            {
                stateTimer_ = 0.0F;
                state_ = alertAfterAction_ ? ChickenState::Alert : ChickenState::Idle;
                return ChickenEvent::EscapeFinished;
            }
            break;
        case ChickenState::Idle:
        case ChickenState::Alert:
        case ChickenState::Hidden: break;
    }
    return ChickenEvent::None;
}

const Texture2D *Chicken::CurrentTexture() const
{
    switch (state_)
    {
        case ChickenState::Alert:
        case ChickenState::Settling: return &suspicious_;
        case ChickenState::Startled: return &startled_;
        case ChickenState::Running: return &run_;
        case ChickenState::Idle: return &idle_;
        case ChickenState::Hidden: return nullptr;
    }
    return &idle_;
}

void Chicken::Draw() const
{
    const Texture2D *texture = CurrentTexture();
    if (texture == nullptr || texture->id == 0) return;

    constexpr float drawHeight = 92.0F;
    constexpr float normalizedFootY = 342.0F / 360.0F;
    const Rectangle destination =
        GroundedDestination(*texture, position_, drawHeight, normalizedFootY);
    DrawGroundShadow(position_, destination.width * 0.29F, 6.0F, Fade(BLACK, 0.24F));
    const Rectangle source = SourceForFacing(*texture, facing_);
    DrawTexturePro(*texture, source, destination, {0.0F, 0.0F}, 0.0F, WHITE);
}

Vector2 Chicken::Position() const
{
    return position_;
}

bool Chicken::CanInteract() const
{
    return state_ == ChickenState::Idle || state_ == ChickenState::Alert;
}

bool Chicken::IsHidden() const
{
    return state_ == ChickenState::Hidden;
}
} // namespace game
