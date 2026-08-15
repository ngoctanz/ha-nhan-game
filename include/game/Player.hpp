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
    Action
};

class Player
{
public:
    bool Load();
    void TriggerAction(CharacterAction action);
    void Update(float deltaTime, bool inputEnabled, const GameInput &input);
    void Draw(const Texture2D *face) const;

    [[nodiscard]] Vector2 Position() const;
    [[nodiscard]] bool FacingRight() const;
    [[nodiscard]] bool IsNear(float worldX, float distance) const;
    [[nodiscard]] const Texture2D *PortraitTexture() const;
    [[nodiscard]] const char *SuggestedFaceExpression() const;

private:
    Animation idle_;
    Animation walk_;
    Animation sneak_;
    Animation goofyWalk_;
    CharacterActionPlayer actions_;
    Vector2 position_ = {290.0F, 622.0F};
    CharacterFacing facing_{FacingDirection::Right};
    bool moving_ = false;
    bool sprinting_ = false;
    PlayerMotion motion_ = PlayerMotion::Idle;
};
} // namespace game
