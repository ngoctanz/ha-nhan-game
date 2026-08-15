#include "game/Player.hpp"
#include "game/Config.hpp"
#include "game/FaceRenderer.hpp"
#include "game/SpritePlacement.hpp"

#include <algorithm>
#include <cmath>
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

    std::vector<std::string> throwPaths;
    for (int i = 1; i <= 4; ++i)
        throwPaths.push_back("assets/characters/player/actions/throw/" +
                             std::string(TextFormat("throw_%02d.png", i)));

    std::vector<std::string> runPaths;
    for (int i = 1; i <= 4; ++i)
        runPaths.push_back("assets/characters/player/actions/run/" +
                           std::string(TextFormat("run_%02d.png", i)));

    std::vector<std::string> flyingKickPaths;
    for (int i = 1; i <= 4; ++i)
        flyingKickPaths.push_back("assets/characters/player/actions/flying_kick/" +
                                  std::string(TextFormat("flying_kick_%02d.png", i)));
    const std::vector<std::string> defeatedPaths = {root + "sit/sit_02.png"};

    const bool idleLoaded = idle_.Load(idlePaths, 1.0F, animateStageCrop, 267, 420);
    const bool walkLoaded = walk_.Load(walkPaths, 10.0F, animateStageCrop, 267, 420);
    const bool sneakLoaded = sneak_.Load(sneakPaths, 9.0F);
    const bool goofyLoaded = goofyWalk_.Load(goofyPaths, 22.0F);
    const bool runLoaded = run_.Load(runPaths, 11.0F);
    const bool throwLoaded = throw_.Load(throwPaths, 9.0F);
    const bool flyingKickLoaded = flyingKick_.Load(flyingKickPaths, 8.0F);
    const bool defeatedLoaded = defeated_.Load(defeatedPaths, 1.0F,
                                               animateStageCrop, 267, 420);
    const bool actionsLoaded = actions_.Load(root + "gesture/", animateStageCrop, 267, 420);
    return idleLoaded && walkLoaded && sneakLoaded && goofyLoaded && runLoaded && throwLoaded &&
           flyingKickLoaded && defeatedLoaded && actionsLoaded;
}

void Player::Place(Vector2 feet, float targetX)
{
    position_ = feet;
    facing_.FaceToward(position_.x, targetX);
    moving_ = false;
    scriptedMove_ = ScriptedMove::None;
    flyingKickActive_ = false;
    defeatedActive_ = false;
    motion_ = PlayerMotion::Idle;
}

void Player::FaceToward(float targetX)
{
    facing_.FaceToward(position_.x, targetX);
}

void Player::TriggerAction(CharacterAction action)
{
    actions_.Trigger(action);
}

void Player::StartThrow(Vector2 target, ThrowProjectileKind projectile)
{
    facing_.FaceToward(position_.x, target.x);
    actions_.Trigger(CharacterAction::None);
    throw_.Reset();
    throwElapsed_ = 0.0F;
    throwTarget_ = target;
    throwProjectile_ = projectile;
    const float direction = facing_.IsRight() ? 1.0F : -1.0F;
    throwStart_ = {position_.x + direction * 55.0F, position_.y - 175.0F};
    throwing_ = true;
    moving_ = false;
    motion_ = PlayerMotion::Throw;
}

void Player::StartGoofyWalkTo(float targetX)
{
    scriptedTargetX_ = targetX;
    facing_.FaceToward(position_.x, targetX);
    actions_.Clear();
    goofyWalk_.Reset();
    scriptedMove_ = ScriptedMove::GoofyWalk;
    defeatedActive_ = false;
}

void Player::StartSneakTo(float targetX)
{
    scriptedTargetX_ = targetX;
    facing_.FaceToward(position_.x, targetX);
    actions_.Clear();
    sneak_.Reset();
    scriptedMove_ = ScriptedMove::Sneak;
    defeatedActive_ = false;
}

void Player::StartRunTo(float targetX)
{
    scriptedTargetX_ = targetX;
    facing_.FaceToward(position_.x, targetX);
    actions_.Clear();
    run_.Reset();
    scriptedMove_ = ScriptedMove::Run;
    defeatedActive_ = false;
}

void Player::StartFlyingKick(float targetX)
{
    facing_.FaceToward(position_.x, targetX);
    actions_.Clear();
    flyingKick_.Reset();
    flyingKickElapsed_ = 0.0F;
    flyingKickStartX_ = position_.x;
    flyingKickTargetX_ = targetX;
    flyingKickActive_ = true;
    scriptedMove_ = ScriptedMove::None;
    defeatedActive_ = false;
}

void Player::SetDefeated(bool defeated)
{
    defeatedActive_ = defeated;
    scriptedMove_ = ScriptedMove::None;
    flyingKickActive_ = false;
    actions_.Clear();
    motion_ = defeated ? PlayerMotion::Defeated : PlayerMotion::Idle;
}

void Player::SetSeated(bool seated)
{
    SetDefeated(seated);
}

PlayerEvent Player::Update(float deltaTime, bool inputEnabled, const GameInput &input)
{
    if (flyingKickActive_)
    {
        constexpr float duration = 0.50F;
        flyingKickElapsed_ += deltaTime;
        flyingKick_.UpdateOnce(deltaTime);
        const float progress = std::clamp(flyingKickElapsed_ / duration, 0.0F, 1.0F);
        position_.x = flyingKickStartX_ + (flyingKickTargetX_ - flyingKickStartX_) * progress;
        motion_ = PlayerMotion::FlyingKick;
        if (progress >= 1.0F)
        {
            flyingKickActive_ = false;
            motion_ = PlayerMotion::Idle;
            return PlayerEvent::FlyingKickHit;
        }
        return PlayerEvent::None;
    }

    if (scriptedMove_ != ScriptedMove::None)
    {
        const float direction = scriptedTargetX_ >= position_.x ? 1.0F : -1.0F;
        facing_.Match(direction > 0.0F);
        const bool sneaking = scriptedMove_ == ScriptedMove::Sneak;
        const bool running = scriptedMove_ == ScriptedMove::Run;
        const float speed = sneaking ? 118.0F : running ? 430.0F : 235.0F;
        Animation &animation = sneaking ? sneak_ : running ? run_ : goofyWalk_;
        motion_ = sneaking ? PlayerMotion::Sneak :
                  running ? PlayerMotion::Run : PlayerMotion::GoofyWalk;
        animation.Update(deltaTime);
        const float nextX = position_.x + direction * speed * deltaTime;
        const bool arrived = direction > 0.0F ? nextX >= scriptedTargetX_ : nextX <= scriptedTargetX_;
        position_.x = arrived ? scriptedTargetX_ : nextX;
        if (arrived)
        {
            scriptedMove_ = ScriptedMove::None;
            motion_ = PlayerMotion::Idle;
            return PlayerEvent::ScriptedMoveFinished;
        }
        return PlayerEvent::None;
    }

    if (defeatedActive_)
    {
        moving_ = false;
        motion_ = PlayerMotion::Defeated;
        return PlayerEvent::None;
    }

    if (throwing_)
    {
        throwElapsed_ += deltaTime;
        throw_.UpdateOnce(deltaTime);
        motion_ = PlayerMotion::Throw;
        constexpr float releaseTime = 2.0F / 9.0F;
        constexpr float flightTime = 0.30F;
        if (throwElapsed_ >= releaseTime + flightTime)
        {
            throwing_ = false;
            motion_ = PlayerMotion::Idle;
            return PlayerEvent::ThrowHit;
        }
        return PlayerEvent::None;
    }

    if (actions_.IsActive())
    {
        moving_ = false;
        motion_ = PlayerMotion::Action;
        return PlayerEvent::None;
    }

    float direction = 0.0F;
    bool sprint = false;
    if (inputEnabled)
    {
        direction = input.moveAxis;
        sprint = input.sprintHeld;
    }

    moving_ = direction != 0.0F;
    if (moving_)
    {
        facing_.Match(direction > 0.0F);
        const bool sneaking = input.sneakHeld;
        float speed = sprint ? 430.0F : 270.0F;
        if (sneaking)
        {
            motion_ = PlayerMotion::Sneak;
            speed = 115.0F;
            sneak_.Update(deltaTime);
        }
        else
        {
            motion_ = PlayerMotion::Walk;
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
    return PlayerEvent::None;
}

void Player::Draw(const Texture2D *face) const
{
    const Animation *animation = &idle_;
    if (motion_ == PlayerMotion::Walk) animation = &walk_;
    else if (motion_ == PlayerMotion::Sneak) animation = &sneak_;
    else if (motion_ == PlayerMotion::GoofyWalk) animation = &goofyWalk_;
    else if (motion_ == PlayerMotion::Run) animation = &run_;
    else if (motion_ == PlayerMotion::Throw) animation = &throw_;
    else if (motion_ == PlayerMotion::FlyingKick) animation = &flyingKick_;
    else if (motion_ == PlayerMotion::Defeated) animation = &defeated_;
    const Texture2D *texturePointer = nullptr;
    if (motion_ == PlayerMotion::Action) texturePointer = actions_.CurrentFrame();
    else if (animation->IsLoaded()) texturePointer = &animation->Current();
    if (texturePointer == nullptr || texturePointer->id == 0) return;
    const Texture2D &texture = *texturePointer;
    // The exported Hà Nhân artwork naturally faces left.
    const Rectangle source = SourceForFacing(texture, facing_, false);
    const bool normalizedAction = motion_ == PlayerMotion::Sneak ||
                                  motion_ == PlayerMotion::GoofyWalk ||
                                  motion_ == PlayerMotion::Run ||
                                  motion_ == PlayerMotion::Throw ||
                                  motion_ == PlayerMotion::FlyingKick;
    float drawHeight = 335.0F;
    if (motion_ == PlayerMotion::Sneak) drawHeight = 258.0F;
    else if (motion_ == PlayerMotion::GoofyWalk) drawHeight = 292.0F;
    else if (motion_ == PlayerMotion::Run) drawHeight = 310.0F;
    else if (motion_ == PlayerMotion::Throw) drawHeight = 300.0F;
    else if (motion_ == PlayerMotion::FlyingKick) drawHeight = 300.0F;
    else if (motion_ == PlayerMotion::Defeated) drawHeight = 280.0F;
    // Standard exports use a 372/420 foot line; normalized movement exports
    // use 340/360.  Both now land on the same world-space GroundY.
    float normalizedFootY = normalizedAction ? 340.0F / 360.0F : 372.0F / 420.0F;
    if (motion_ == PlayerMotion::FlyingKick) normalizedFootY = 340.0F / 381.0F;
    else if (motion_ == PlayerMotion::Run) normalizedFootY = 480.0F / 512.0F;
    else if (motion_ == PlayerMotion::Defeated) normalizedFootY = 0.86F;
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
    else if (motion_ == PlayerMotion::Run)
    {
        static constexpr FaceAnchor anchors[] = {
            {0.429F, 0.383F, 0.26F, 0.18F, Color{250, 250, 247, 255}, 0.0F, false},
            {0.405F, 0.570F, 0.26F, 0.18F, Color{250, 250, 247, 255}, 0.0F, false},
            {0.459F, 0.463F, 0.26F, 0.18F, Color{250, 250, 247, 255}, 0.0F, false},
            {0.422F, 0.397F, 0.26F, 0.18F, Color{250, 250, 247, 255}, 0.0F, false}};
        faceAnchor = anchors[run_.CurrentIndex()];
    }
    else if (motion_ == PlayerMotion::Throw)
    {
        static constexpr FaceAnchor anchors[] = {
            // Each keyframe moves the head independently. Keep a measured
            // anatomical center per frame instead of pinning the face to the
            // animation canvas center.
            {0.453F, 0.308F, 0.20F, 0.18F, Color{250, 250, 247, 255}, 0.0F, false},
            {0.419F, 0.326F, 0.20F, 0.18F, Color{250, 250, 247, 255}, 0.0F, false},
            {0.503F, 0.311F, 0.20F, 0.18F, Color{250, 250, 247, 255}, 0.0F, false},
            {0.391F, 0.382F, 0.20F, 0.18F, Color{250, 250, 247, 255}, 6.0F, false}};
        faceAnchor = anchors[throw_.CurrentIndex()];
    }
    else if (motion_ == PlayerMotion::FlyingKick)
    {
        static constexpr FaceAnchor anchors[] = {
            {0.44F, 0.47F, 0.30F, 0.24F, Color{250, 250, 247, 255}, 0.0F, false},
            {0.44F, 0.38F, 0.30F, 0.24F, Color{250, 250, 247, 255}, 0.0F, false},
            {0.44F, 0.38F, 0.30F, 0.24F, Color{250, 250, 247, 255}, 0.0F, false},
            {0.44F, 0.49F, 0.30F, 0.24F, Color{250, 250, 247, 255}, 0.0F, false}};
        faceAnchor = anchors[flyingKick_.CurrentIndex()];
    }
    else if (motion_ == PlayerMotion::Defeated)
        faceAnchor = {0.43F, 0.314F, 0.24F, 0.15F, Color{250, 250, 247, 255}};
    else
        faceAnchor = {0.43F, 0.314F, 0.24F, 0.15F, Color{250, 250, 247, 255}};
    // The goofy walk was extracted from the supplied GIF and already contains
    // its intended meme face. Keep that artwork intact instead of stacking a
    // second FaceLibrary expression on top of it.
    if (motion_ != PlayerMotion::GoofyWalk)
        DrawFaceOverlay(face, destination, faceAnchor, facing_.IsFlipped(false));

    constexpr float releaseTime = 2.0F / 9.0F;
    constexpr float flightTime = 0.30F;
    if (throwing_ && throwElapsed_ >= releaseTime)
    {
        const float progress = std::clamp((throwElapsed_ - releaseTime) / flightTime, 0.0F, 1.0F);
        const Vector2 rock = {
            throwStart_.x + (throwTarget_.x - throwStart_.x) * progress,
            throwStart_.y + (throwTarget_.y - throwStart_.y) * progress};
        if (throwProjectile_ == ThrowProjectileKind::CornCake)
        {
            const Vector2 flight = {throwTarget_.x - throwStart_.x,
                                    throwTarget_.y - throwStart_.y};
            const float length = std::max(1.0F, std::sqrt(flight.x * flight.x +
                                                          flight.y * flight.y));
            const Vector2 backward = {-flight.x / length, -flight.y / length};
            for (int i = 5; i >= 1; --i)
            {
                const float distance = static_cast<float>(i) * 11.0F;
                const Vector2 ember = {rock.x + backward.x * distance,
                                       rock.y + backward.y * distance};
                DrawCircleV(ember, 3.0F + (6 - i) * 0.9F,
                            Fade(i > 3 ? ORANGE : GOLD, (6 - i) * 0.12F));
            }
            DrawCircleV(rock, 23.0F, Fade(ORANGE, 0.20F));
            DrawCircleV(rock, 17.0F, Fade(GOLD, 0.28F));
            DrawEllipse(static_cast<int>(rock.x), static_cast<int>(rock.y),
                        15.0F, 9.0F, Color{218, 167, 62, 255});
            DrawEllipseLines(static_cast<int>(rock.x), static_cast<int>(rock.y),
                             15.0F, 9.0F, Color{113, 73, 27, 255});
            DrawLineEx({rock.x - 7, rock.y}, {rock.x + 7, rock.y}, 2.0F,
                       Color{151, 101, 34, 255});
        }
        else
        {
            DrawCircleV(rock, 10.0F, Color{92, 82, 70, 255});
            DrawCircleV({rock.x - 3.0F, rock.y - 3.0F}, 3.0F,
                        Color{145, 132, 113, 255});
        }
    }
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
        case PlayerMotion::Sneak: return "knowing_smirk";
        case PlayerMotion::GoofyWalk: return "surprised";
        case PlayerMotion::Run: return "terrified";
        case PlayerMotion::Walk: return "smug";
        case PlayerMotion::Action: return actions_.SuggestedFace();
        case PlayerMotion::Idle: return "blank_stare";
    }
    return "blank_stare";
}
} // namespace game
