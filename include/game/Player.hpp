#pragma once

#include "game/Animation.hpp"
#include "game/CharacterAction.hpp"
#include "game/CharacterFacing.hpp"
#include "game/GameInput.hpp"

#include <raylib.h>

namespace game
{
enum class PlayerMotion
{
    Idle,
    Walk,
    Sneak,
    GoofyWalk,
    Run,
    Action,
    Throw,
    FlyingKick,
    Defeated
};

enum class PlayerEvent
{
    None,
    ThrowHit,
    ScriptedMoveFinished,
    FlyingKickHit
};

enum class ThrowProjectileKind
{
    Rock,
    CornCake
};

class Player
{
public:
    bool Load();
    void Place(Vector2 feet, float targetX);
    void FaceToward(float targetX);
    void TriggerAction(CharacterAction action);
    void StartThrow(Vector2 target, ThrowProjectileKind projectile = ThrowProjectileKind::Rock);
    void StartGoofyWalkTo(float targetX);
    void StartSneakTo(float targetX);
    void StartRunTo(float targetX);
    void StartFlyingKick(float targetX);
    void SetSeated(bool seated);
    void SetDefeated(bool defeated);
    PlayerEvent Update(float deltaTime, bool inputEnabled, const GameInput &input);
    void Draw(const Texture2D *face) const;

    [[nodiscard]] Vector2 Position() const;
    [[nodiscard]] bool FacingRight() const;
    [[nodiscard]] bool IsNear(float worldX, float distance) const;
    [[nodiscard]] const Texture2D *PortraitTexture() const;
    [[nodiscard]] const char *SuggestedFaceExpression() const;

private:
    enum class ScriptedMove
    {
        None,
        GoofyWalk,
        Sneak,
        Run
    };

    Animation idle_;
    Animation walk_;
    Animation sneak_;
    Animation goofyWalk_;
    Animation run_;
    Animation throw_;
    Animation flyingKick_;
    Animation defeated_;
    CharacterActionPlayer actions_;
    Vector2 position_ = {290.0F, 622.0F};
    CharacterFacing facing_{FacingDirection::Right};
    bool moving_ = false;
    bool throwing_ = false;
    float throwElapsed_ = 0.0F;
    Vector2 throwStart_ = {};
    Vector2 throwTarget_ = {};
    ThrowProjectileKind throwProjectile_ = ThrowProjectileKind::Rock;
    ScriptedMove scriptedMove_ = ScriptedMove::None;
    float scriptedTargetX_ = 0.0F;
    bool flyingKickActive_ = false;
    float flyingKickElapsed_ = 0.0F;
    float flyingKickStartX_ = 0.0F;
    float flyingKickTargetX_ = 0.0F;
    bool defeatedActive_ = false;
    PlayerMotion motion_ = PlayerMotion::Idle;
};
} // namespace game
